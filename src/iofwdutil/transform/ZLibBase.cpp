#include <boost/format.hpp>

#include "TransformException.hh"
#include "ZLibBase.hh"
#include "iofwdutil/assert.hh"

using boost::format;

namespace iofwdutil
{
   namespace transform
   {
      //=======================================================================

      ZLibBase::ZLibBase ()
         : isInit_(false),
           first_call_ (true)
      {
      }

      ZLibBase::~ZLibBase ()
      {
      }

      void ZLibBase::reset ()
      {
         curstate_ = SUPPLY_INBUF;
         out_used_ = 0;

         stream_.zalloc = 0;
         stream_.zfree = 0;
         stream_.opaque = 0;

         first_call_ = true;
         first_process_ = true;

         if (isInitialized ())
         {
            done ();
         }
         check (init());
      }

      int ZLibBase::check (int ret) const
      {
         if (ret == Z_OK)
            return ret;
         switch (ret)
         {
            case Z_MEM_ERROR:
            case Z_VERSION_ERROR:
            case Z_STREAM_ERROR:
               ZTHROW (TransformException ()
                     << zexception_msg(str(format("ZLib error: %s") %
                        stream_.msg)));
            default:
               ZTHROW (TransformException ()
                     << zexception_msg("Unknown ZLib error!"));
         }
      }

      void ZLibBase::transform (const void * const in, size_t insize,
            void * out, size_t outsize, size_t * written, int * outstate,
            bool flushflag)
      {
         // After transform returns TRANSFORM_DONE, reset must be called
         // before trying to transform more data.
         ALWAYS_ASSERT(curstate_ != TRANSFORM_DONE);

         *written = 0;

         if (!isInitialized ())
         {
            stream_.zalloc = 0;
            stream_.zfree = 0;
            stream_.opaque = 0;
            init ();
         }

         if (first_call_)
         {
            stream_.next_in = (Bytef*) in;
            stream_.avail_in = insize;
            stream_.next_out = (Bytef*) out;
            stream_.avail_out = outsize;
            first_call_ = false;
         }

         if (curstate_ == SUPPLY_INBUF)
         {
            stream_.next_in = (Bytef*)in;
            stream_.avail_in = insize;
         }
         else if (curstate_ == CONSUME_OUTBUF)
         {
            stream_.next_out = (Bytef*) out;
            stream_.avail_out = outsize;
            out_used_ = 0;
         }

         do
         {
            if (!stream_.avail_out)
            {
               curstate_ = CONSUME_OUTBUF;
               *written = outsize - stream_.avail_out;
               *outstate = curstate_;
               return;
            }

            if (!stream_.avail_in && !flushflag)
            {
               curstate_ = SUPPLY_INBUF;
               *outstate = curstate_;
               return;
            }

            ALWAYS_ASSERT (stream_.avail_in || stream_.avail_out);

            // Specifying Z_FINISH to the first call to inflate/deflate has a
            // special meaning: it guarantees we have enough output space to
            // output everything in one call to inflate/deflate. Since we
            // cannot guarantee this, we have to make sure we don't pass
            // Z_FINISH on the first call to process.
            const int ret = process (flushflag);

            first_process_ = false;

            // Z_BUF_ERROR indicates a program error (shouldn't call zlib
            // without something todo)
            ALWAYS_ASSERT(ret != Z_BUF_ERROR);

            switch (ret)
            {
               case Z_STREAM_END:
                  curstate_ = TRANSFORM_DONE;
                  *outstate = curstate_;
                  *written = outsize - stream_.avail_out;
                  check(done ());
                  return;
               case Z_OK:
                  break;
               default:
                  check (ret);
                  break;
            }
         } while (true);

     }

      state ZLibBase::getTransformState () const
      {
         return curstate_;
      }


      //=======================================================================
   }
}
