#include "LZFCompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "TransformException.hh"

extern "C" {
#include "src/c-util/transform/lzf/lzf.h"
}

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTEncode,
      iofwdutil::transform::LZFCompress,
      "LZF",
      lzfencode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      LZFCompress::LZFCompress ()
         : iostate_read_(LZF_BLOCK_SIZE),
         iostate_write_(LZF_BLOCK_SIZE)
      {
         reset ();
      }

      LZFCompress::~LZFCompress ()
      {

      }

      void LZFCompress::reset ()
      {
         // clear internal state
         block_compressed_ = false;
         internal_state_ = COMPRESSING_GETINPUT;
         curstate_ = SUPPLY_INBUF;
         iostate_read_.reset ();
         iostate_write_.reset ();
         update_in_ = true;
         update_out_ = true;
      }

      void LZFCompress::transform (const void * const in, size_t maxinsize,
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
               case COMPRESSING_GETINPUT:
                  {
                     compress_input_ = 0;
                     compress_insize_ = 0;
                     compress_output_ = 0;
                     compress_outsize_ = 0;
                     block_size_ = 0;

                     compress_insize_ = LZF_BLOCK_SIZE - LZF_HEADER_SIZE;

                     if (flushflag)
                     {
                        // if the user indicates that this is the last of the
                        // data, don't request more than we have
                        compress_insize_ = std::min(compress_insize_,
                              iostate_read_.getAvail ());
                     }

                     // we compress up to LZF_BLOCK_SIZE bytes but need to
                     // subtract the header size
                     compress_input_ = static_cast<const char*>
                        (iostate_read_.reserve (compress_insize_));

                     // If we have the flush flag on, this should not fail
                     ALWAYS_ASSERT(!flushflag || compress_input_);

                     if (!compress_input_)
                     {
                        // don't have enough for input; change state to supply
                        // more input
                        curstate_ = SUPPLY_INBUF;
                        *outstate = curstate_;
                        update_in_ = true;
                        return;
                     }

                     internal_state_ = COMPRESSING_GETOUTPUT;
                  }
                  break;
               case COMPRESSING_GETOUTPUT:
                  {
                     // we need space for the data + header
                     compress_outsize_ = compress_insize_ + LZF_HEADER_SIZE;
                     ASSERT(compress_outsize_ <= LZF_BLOCK_SIZE);

                     // Get up to LZF_BLOCK_SIZE output
                     compress_output_ = static_cast<char*>
                        (iostate_write_.reserve (compress_outsize_));

                     // write reserve cannot fail
                     ASSERT(compress_output_);

                     internal_state_ = COMPRESSING;
                  }
                  break;
               case COMPRESSING:
                  {
                     char * data_start = compress_output_ + LZF_HEADER_SIZE;
                     const size_t max_data_size = compress_outsize_ -
                        (data_start - compress_output_);

                     // try to compress and see if size reduces enough
                     size_t data_size = lzf_compress (
                           compress_input_, compress_insize_,
                           data_start, max_data_size);

                     const bool store_compressed =
                        (data_size && data_size < compress_insize_);

                     if (!store_compressed)
                     {
                        // block didn't compress well; store uncompressed
                        ASSERT(compress_insize_ <= max_data_size);
                        memcpy (data_start, compress_input_, compress_insize_);
                        data_size = compress_insize_;
                     }

                     // Fill in header
                     char * headerout = compress_output_;
                     *headerout++ = (store_compressed ?
                           HEADER_BLOCK_COMPRESSED : HEADER_BLOCK_UNCOMPRESSED);
                     writer_.reset (headerout, data_start - headerout);

                     ALWAYS_ASSERT(((data_start - headerout) >= 4));

                     const uint32_t encodedsize = data_size;
                     // in case the above truncates
                     ASSERT(encodedsize == data_size);       
                     process (writer_, encodedsize);

                     block_size_ = data_size + LZF_HEADER_SIZE;
                     ASSERT((data_start - compress_output_) == LZF_HEADER_SIZE);

                     // Now we have a block of 'blocksize' starting at
                     // compress_output_; we need to flush that, and flush the
                     // input
                     internal_state_ = FLUSH_INPUT;
                  }
                  break;
               case FLUSH_INPUT:
                  {
                     iostate_read_.release (compress_insize_);
                     compress_insize_ = 0;
                     compress_input_ = 0;

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
                     internal_state_ = COMPRESSING_GETINPUT;
                     
                     // At this point we flushed the block; If there are no
                     // more blocks in the input, and the user set the flush
                     // flag, we're done.
                     if (flushflag & iostate_read_.empty())
                     {
                        internal_state_ = FLUSHED;
                        curstate_ = TRANSFORM_DONE;
                        *outstate = curstate_;
                        *written = iostate_write_.getWritten ();
                        return;
                     }
                  }
                  break;
               case FLUSHED:
                  {
                     ZTHROW (TransformException ()
                           << zexception_msg("Reset needs to be called "
                           "before compressing more data after flush!"));
                  }
                  break;
               default:
                  ALWAYS_ASSERT(false && "Invalid internal state");
            }
         }
      }


      state LZFCompress::getTransformState () const
      {
         return curstate_;
      }

      //=====================================================================
   }
}
