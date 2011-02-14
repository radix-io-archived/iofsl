#include "BMIInput.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdutil/IOFWDLog.hh"

#include <boost/format.hpp>

using boost::format;

namespace net
{
   namespace bmi
   {
      //=====================================================================

      /**
       * Constructor for outgoing connection.
       * We have to decide on the cookie and max packet size
       */
      BMIInput::BMIInput (
            iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            BMI_addr_t addr,
            bmi_msg_tag_t tag,
            const BMIStreamHeader & h)
         : log_(log),
           bmiresource_(bmi),
           endpoint_(addr),
           tag_(tag),
           send_open_ (false),
           cur_read_raw_ (0),
           cur_read_raw_size_ (0),
           seq_ (0)
      {
         info_.buffer = 0;
         info_.addr = 0;
         cookie_ = h.cookie;
         max_packet_ = h.packet_size;
         ALWAYS_ASSERT(!h.flags);
         ALWAYS_ASSERT(h.version == 1);
      }

      /**
       * Constructor for incoming connections. In this case, the endpoint
       * knows we're going to respond, so we consider the send part of the
       * connection to be open.
       */
      BMIInput::BMIInput (iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            const BMI_unexpected_info & info,
            bmi_msg_tag_t tag)
         : log_(log),
           bmiresource_ (bmi),
           endpoint_ (info.addr),
           tag_ (tag),
           send_open_ (true),
           info_ (info),
           cookie_ (0),
           cur_read_raw_ (info_.buffer),
           cur_read_raw_size_ (info_.size),
           seq_ (0)
      {
         // decode header
         BMIStreamHeader header;
         cur_read_raw_used_ = header.decode (cur_read_raw_, cur_read_raw_size_);
         cookie_ = header.cookie;
         max_packet_ = header.packet_size;
         ALWAYS_ASSERT(!header.flags);
         ALWAYS_ASSERT(!header.version);
      }

      BMIInput::~BMIInput ()
      {
         // Free current buffer if it isn't the unexpected receive
         if (info_.buffer != cur_read_raw_ && cur_read_raw_)
         {
            BMI_memfree (endpoint_, cur_read_raw_,
                  cur_read_raw_size_, BMI_RECV);
         }

         // Free unexpected receive buffer if any
         if (info_.buffer)
            BMI_unexpected_free (info_.addr, info_.buffer);
      }

      
      void BMIInput::read (const void ** ptr, size_t * size,
            const iofwdevent::CBType & cb)
      {
         // Check if we need to free the old buffer
         if (cur_read_raw_)
         {
            // Check if there is unconsumed data in there (would happen for
            // unexpected receive for example)
            if (cur_read_raw_used_ < cur_read_raw_size_)
            {
               *ptr = static_cast<const char*>(cur_read_raw_) +
                  cur_read_raw_used_;
               *size = cur_read_raw_size_ - cur_read_raw_used_;

               // Mark all data as used
               cur_read_raw_used_ += *size;

               cb (iofwdevent::CBException ());

               return;
            }

            // All data consumed, cleanup old buffer

            // if we have an unexpected buffer
            if (info_.buffer)
            {
               ALWAYS_ASSERT(cur_read_raw_ == info_.buffer);
               BMI_unexpected_free (info_.addr, info_.buffer);
               info_.addr = 0; info_.buffer = 0;
               cur_read_raw_ = 0; cur_read_raw_size_ =0;
            }
            else
            {
               BMI_memfree (endpoint_, cur_read_raw_,
                     cur_read_raw_buffer_size_, BMI_RECV);
               cur_read_raw_ = 0;
               cur_read_raw_size_ = 0;
               cur_read_raw_buffer_size_ = 0;
            }
         }
         else
         {
            // No need to check for unexpected info, the constructor already
            // took care of that.
         }

         ALWAYS_ASSERT(!cur_read_raw_);

         // OK, need to post a receive
         cur_read_raw_ = BMI_memalloc (endpoint_, max_packet_, BMI_RECV);
         cur_read_raw_buffer_size_ = max_packet_;

         ALWAYS_ASSERT(cur_read_raw_);
         cur_read_raw_size_ = max_packet_;
         cur_read_cb_ = cb;
         user_read_ptr_ = ptr;
         user_read_size_ = size;

         // Post receive
         postReceive ();
      }

      void BMIInput::postReceive ()
      {
         ZLOG_DEBUG(log_, format("posting receive (max=%u) on tag %u")
               % cur_read_raw_size_ % tag_);

         // Repost receive
         bmiresource_.post_recv (
               boost::bind (&BMIInput::readComplete, this, _1),
               endpoint_,
               cur_read_raw_,
               cur_read_raw_size_,
               &cur_read_raw_actual_,
               BMI_PRE_ALLOC,
               tag_,
               0);

      }

      void BMIInput::readComplete (const iofwdevent::CBException & e)
      {
         e.check ();

         // We got incoming data on this connection:
         cur_read_raw_size_ = cur_read_raw_actual_;

         ZLOG_DEBUG (log_, format("read completed: tag=%u bytes=%u")
               % tag_ % cur_read_raw_size_);

         // Check if this is a BMI Stream message
         try
         {
            BMIPacketHeader p;
            cur_read_raw_used_ = p.decode (cur_read_raw_, cur_read_raw_size_);

            // Check cookie & sequence
            if (p.cookie != cookie_)
            {
               ZLOG_INFO (log_, format("Invalid cookie in receive! "
                     "Expected %lu, got %lu. Dropped packet!")
                     % p.cookie % cookie_);
               ALWAYS_ASSERT(false);
            }
            if (p.seq != seq_)
            {
               ZLOG_INFO (log_, format("Invalid sequence in packet! "
                     "(got %u, expected %u) dropped packet!") 
                     % p.seq % seq_);
               ALWAYS_ASSERT(false);
            }

            // Header checks out OK
            ++seq_;

            // All OK, this packet is for our connection
            *user_read_ptr_ = static_cast<const char *>(cur_read_raw_) +
               cur_read_raw_used_;
            *user_read_size_ = cur_read_raw_size_ - cur_read_raw_used_;
            cur_read_raw_used_ = cur_read_raw_size_;

            // Call callback
            iofwdevent::CBType cb;
            cb.swap (cur_read_cb_);
            cb (iofwdevent::CBException ());
         }
         catch (...)
         {
            // could get here if 
            // - header fails to decode
            // - message is too small;
            // - cookie wrong
            //
            // need to repost receive
            ZLOG_INFO (log_, "Reposting receive...");
            postReceive ();
         }
      }

      //=====================================================================
   }
}
