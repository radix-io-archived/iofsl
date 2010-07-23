#include "CopyTransform.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "TransformException.hh"

// @TODO: add tags to factory so that the GenericTransform library can be
// split in a 'encode' and 'decode' factory.
GENERIC_FACTORY_CLIENT(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::CopyTransform,
      "copy",
      copytransform);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      CopyTransform::CopyTransform ()
         : iostate_read_(BLOCKSIZE),
         iostate_write_(BLOCKSIZE)
      {
         reset ();
      }

      CopyTransform::~CopyTransform ()
      {

      }

      void CopyTransform::reset ()
      {
         // clear internal state
         internal_state_ = GETINPUT;
         curstate_ = SUPPLY_INBUF;
         iostate_read_.reset ();
         iostate_write_.reset ();
         update_in_ = true;
         update_out_ = true;
      }

      void CopyTransform::transform (const void * const in, size_t maxinsize,
            void * out, size_t maxoutsize, size_t * written,
            int * outstate, bool flushflag)
      {
         *written = 0;

         if (update_in_)
         {
            iostate_read_.update (in, maxinsize);
            update_in_ = false;
         }

         if (update_out_)
         {
            iostate_write_.update (out, maxoutsize);
            update_out_ = false;
         }


         while (true)
         {
            switch (internal_state_)
            {
               case GETINPUT:
                  {
                     block_size_ = BLOCKSIZE;
                     ALWAYS_ASSERT(block_size_);

                     // Note: there is a weird case here: if the user has 2
                     // input buffers, in total less than BLOCKSIZE and an
                     // output buffer large enough for all the data.
                     //
                     // The first time, flushflag is not set and an attempt
                     // will be made to reserve BLOCKSIZE. As the total of the
                     // input buffers is less than BLOCKSIZE, this will fail.
                     // The second buffer will be given, and this time the
                     // flushflag will be set, causing the code below to retry
                     // the reserve but with less than BLOCKSIZE this time.
                     // (violating the IOReadBuffer rule that on a retry, the
                     // same amount needs to be specified)

                     if (flushflag)
                     {
                        block_size_ = std::min(block_size_,
                              iostate_read_.getAvail());
                     }

                     copy_input_ = static_cast<const
                        char*>(iostate_read_.reserve (block_size_));

                     if (!copy_input_)
                     {
                        curstate_ = SUPPLY_INBUF;
                        *outstate = curstate_;
                        update_in_ = true;
                        return;
                     }

                     internal_state_ = GETOUTPUT;
                  }
                  break;
               case GETOUTPUT:
                  {
                     copy_output_ = iostate_write_.reserve (block_size_);

                     // write reserve does not fail.
                     ALWAYS_ASSERT(copy_output_);
                     internal_state_ = COPYING;
                  }
                  break;
               case COPYING:
                  {
                     memcpy (copy_output_, copy_input_,
                           block_size_);
                     internal_state_ = FLUSH_INPUT;
                  }
                  break;
              case FLUSH_INPUT:
                  {
                     // flushInput cannot fail
                     iostate_read_.release (block_size_);
                     internal_state_ = FLUSH_OUTPUT;
                  }
                  break;
               case FLUSH_OUTPUT:
                  {
                     if (!iostate_write_.release (block_size_))
                     {
                        // output buffer full; update written and
                        // status
                        curstate_ = CONSUME_OUTBUF;
                        *written = maxoutsize;
                        *outstate = curstate_;
                        update_out_ = true;
                        return;
                     }

                     // If the user indicates a flush and we don't have any
                     // more input data, we're done.
                     if (flushflag && iostate_read_.empty ())
                     {
                        curstate_ = TRANSFORM_DONE;
                        *written = iostate_write_.getWritten ();
                        *outstate = curstate_;
                        internal_state_ = FLUSHED;
                        return;
                     }

                     internal_state_ = GETINPUT;
                  }
                  break;

               case FLUSHED:
                  {
                     throw TransformException ("Reset needs to be called "
                           "before transforming more data after flush!");
                  }
                  break;
               default:
                  ALWAYS_ASSERT(false && "Invalid internal state");
            }
         }
      }


      state CopyTransform::getTransformState () const
      {
         return curstate_;
      }

      //=====================================================================
   }
}
