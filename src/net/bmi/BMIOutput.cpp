#include "BMIOutput.hh"
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
      BMIOutput::BMIOutput (
            iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            BMI_addr_t addr,
            bmi_msg_tag_t tag,
            bool incoming,
            const BMIStreamHeader & h)
         : log_(log),
           bmiresource_(bmi),
           endpoint_(addr),
           tag_(tag),
           send_open_ (incoming),
           cur_send_ (0),
           cur_send_size_ (0),
           cur_send_used_ (0),
           seq_ (0),
           reuse_ (false)
      {
         cookie_ = h.cookie;
         int dummy;
         BMI_get_info (endpoint_, BMI_CHECK_MAXSIZE, &dummy);
         max_expected_packet_ = dummy;
         BMI_get_info (endpoint_, BMI_GET_UNEXP_SIZE, &dummy);
         max_unexpected_packet_ = dummy;
         max_packet_ = std::min (
               size_t (h.packet_size),
               size_t(max_expected_packet_));
      }


      BMIOutput::~BMIOutput ()
      {
         // Free send buffer if any
         if (cur_send_)
            BMI_memfree (endpoint_, cur_send_, cur_send_size_, BMI_SEND);
      }

      size_t BMIOutput::getMaxSend () const
      {
         if (send_open_)
            return max_packet_;
         else
            return std::min (size_t(max_unexpected_packet_), size_t(max_packet_));
      }

      void BMIOutput::allocWrite (void ** ptr, size_t * size,
            size_t suggested)
      {
         if (reuse_)
         {
            // Optimization in case somebody called flush on an empty stream
            *ptr = static_cast<char*>(cur_send_) + cur_send_used_;
            *size = cur_send_size_ - cur_send_used_;
            reuse_ = false;
            return;
         }

         // Cannot call allocWrite before sending off previous buffer
         ALWAYS_ASSERT (!cur_send_);

         const size_t max_send = getMaxSend ();

         // Increase buffer size to make room for cookie
         if (!send_open_ && suggested)
            suggested += BMIStreamHeader::getEncodedSize ();

         if (suggested)
            suggested += BMIPacketHeader::getEncodedSize ();

         const size_t nextsize = suggested ?
               std::min (suggested, max_send)
             : max_send;

        cur_send_  = BMI_memalloc (endpoint_, nextsize, BMI_SEND);
        cur_send_size_ = nextsize;

        cur_send_used_ = 0;

        if (!send_open_)
        {
           // stream header
           BMIStreamHeader h;
           h.version = 0;
           h.cookie = cookie_;
           h.packet_size = max_packet_;
           h.flags = 0;
           cur_send_used_ = h.encode (cur_send_, cur_send_size_);
        }
        else
        {
           // normal packet header
           BMIPacketHeader h;
           h.cookie = cookie_;
           h.seq = seq_++;

           cur_send_used_ = h.encode (cur_send_, cur_send_size_);
        }

        *ptr = static_cast<char*>(cur_send_) + cur_send_used_;
        *size = cur_send_size_ - cur_send_used_;
      }

      /**
       * Could modify here: no need to wait for flush to complete,
       * but would need to add some mem limiting to avoid runaway sender...
       */
      void BMIOutput::flush (size_t size, const iofwdevent::CBType & cb)
      {
         if (!size)
         {
            // Optimization: don't flush 0-byte payload packets
            cb (iofwdevent::CBException ());
            reuse_ = true;
            return;
         }

         // cur_send_used_ is the header size
         cur_send_used_ += size;

         // we need to make a copy in case the callback completes
         // immediately and to prevent double free in case 
         void * ptr = cur_send_;
         size_t allocsize = cur_send_size_;
         size_t used = cur_send_used_;
         cur_send_ = 0;
         cur_send_size_ = 0;
         cur_send_used_ = 0;
 
         ZLOG_DEBUG (log_, format("sending %u bytes to tag %u (expec=%i)") %
               used % tag_ % send_open_ );
        
         bool send_open = send_open_;
         send_open_ = true;


         // Note: we bind the buffer size, not the used size since BMI_memfree
         // needs the former
         if (send_open)
         {
            bmiresource_.post_send (
                  boost::bind (&BMIOutput::writeDone, this, ptr,
                     allocsize, cb, _1),
                  endpoint_,
                  ptr,
                  used,
                  BMI_PRE_ALLOC,
                  tag_,
                  0);
         }
         else
         {
           bmiresource_.post_sendunexpected (
                  boost::bind (&BMIOutput::writeDone, this, ptr,
                     allocsize, cb, _1),
                  endpoint_,
                  ptr,
                  used,
                  BMI_PRE_ALLOC,
                  tag_,
                  0);
         }
      }

      void BMIOutput::writeDone (void * sendptr, size_t allocsize,
            const iofwdevent::CBType & cb, const iofwdevent::CBException & e)
      {
         BMI_memfree (endpoint_, sendptr, allocsize, BMI_SEND);
         cb (e);
      }

      //=====================================================================
   }
}
