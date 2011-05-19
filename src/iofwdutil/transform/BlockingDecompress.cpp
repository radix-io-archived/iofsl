#include "BlockingDecompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "TransformException.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      BlockingDecompress::BlockingDecompress ()
         : iostate_read_(BLOCK_SIZE),
         iostate_write_(BLOCK_SIZE)
      {
         reset ();
      }

      BlockingDecompress::~BlockingDecompress ()
      {

      }

      void BlockingDecompress::reset ()
      {
         // clear internal state
         block_compressed_ = false;
         internal_state_ = READING_HEADER;
         curstate_ = SUPPLY_INBUF;
         iostate_read_.reset ();
         iostate_write_.reset ();
         update_in_ = true;
         update_out_ = true;
      }

      void BlockingDecompress::transform (const void * const in, size_t maxinsize,
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
               case READING_HEADER:
                  {
                     decompress_input_ = 0;
                     decompress_output_ = 0;

                     const char * input = static_cast<const
                        char*>(iostate_read_.reserve (HEADER_SIZE));
                     if (!input)
                     {
                        // don't have enough for input; change state to supply
                        // more input
                        curstate_ = SUPPLY_INBUF;
                        *outstate = curstate_;
                        update_in_ = true;
                        return;
                     }

                     switch (input[0])
                     {
                        case HEADER_BLOCK_UNCOMPRESSED:
                           block_compressed_ = false;
                           break;
                        case HEADER_BLOCK_COMPRESSED:
                           block_compressed_ = true;
                           break;
                        default:
                           ZTHROW (TransformException ()
                            << zexception_msg ("Invalid header in"
                               " stream!"));
                     }
                     // skip byte
                     ++input;
                     reader_.reset (input, HEADER_SIZE-1);
                     uint32_t size;
                     process (reader_, size);

                     if (size > BLOCK_SIZE)
                     {
                        ZTHROW (TransformException ()
                              << zexception_msg("Invalid header in stream!"));
                     }

                     block_size_ = size;

                     // no longer need header
                     iostate_read_.release (HEADER_SIZE);

                     internal_state_ = DECOMPRESSING_GETINPUT;
                  }
                  break;
               case DECOMPRESSING_GETINPUT:
                  {
                     ALWAYS_ASSERT(block_size_);

                     decompress_input_ = static_cast<const
                        char*>(iostate_read_.reserve (block_size_));

                     if (!decompress_input_)
                     {
                        curstate_ = SUPPLY_INBUF;
                        *outstate = curstate_;
                        update_in_ = true;
                        return;
                     }

                     internal_state_ = DECOMPRESSING_GETOUTPUT;
                  }
                  break;
               case DECOMPRESSING_GETOUTPUT:
                  {
                     // if we decompress, we need up to BLOCK_SIZE space
                     // otherwise we just copy so we know the exact size
                     // requirement.
                     decompress_output_ = iostate_write_.reserve
                        (block_compressed_ ?
                             static_cast<size_t>(BLOCK_SIZE)
                           : block_size_);

                     // write reserve does not fail.
                     ALWAYS_ASSERT(decompress_output_);

                     internal_state_ = (block_compressed_ ? DECOMPRESSING :
                           COPYING);
                  }
                  break;
               case COPYING:
                  {
                     memcpy (decompress_output_, decompress_input_,
                           block_size_);
                     output_size_ = block_size_;
                     internal_state_ = FLUSH_INPUT;
                  }
                  break;
               case DECOMPRESSING:
                  {
                     ASSERT(block_compressed_);
                     output_size_ = doDecompress (
                           decompress_input_, block_size_,
                           decompress_output_, BLOCK_SIZE);

                     if (!output_size_)
                        ZTHROW (TransformException ()
                              << zexception_msg("Error in decompress"));

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
                     if (!iostate_write_.release (output_size_))
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
                     if (iostate_read_.empty ())
                     {
                        curstate_ = CONSUME_OUTBUF;
                        *written = iostate_write_.getWritten ();
                        *outstate = curstate_;
                        update_out_ = true;
                        //internal_state_ = READING_HEADER;
                        return;
                     }

                     internal_state_ = READING_HEADER;
                  }
                  break;

               case FLUSHED:
                  {
                     ZTHROW (TransformException ()
                           << zexception_msg("Reset needs to be called "
                           "before decompressing more data after flush!"));
                  }
                  break;
               default:
                  ALWAYS_ASSERT(false && "Invalid internal state");
            }
         }
      }


      state BlockingDecompress::getTransformState () const
      {
         return curstate_;
      }

      //=====================================================================
   }
}
