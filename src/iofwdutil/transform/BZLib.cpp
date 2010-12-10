#include <boost/format.hpp>

#include "iofwdutil/assert.hh"
#include "BZLib.hh"
#include "TransformException.hh"

using boost::format;

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      BZLib::BZLib (bool compress)
         : compress_(compress),
           stream_init_ (false)
      {
         reset ();
      }

      BZLib::~BZLib ()
      {
         if (stream_init_)
         {
            BZ2_bzCompressEnd (&stream_);
            stream_init_ = false;
         }
      }

      int BZLib::check (int ret) const
      {
         if (ret == BZ_OK)
            return ret;
         throw TransformException (str(format("BZLib error %i") % ret));
      }

      int BZLib::process (bool flush)
      {
         if (compress_)
         {
            return BZ2_bzCompress (&stream_, (flush ? BZ_FINISH : BZ_RUN));
         }
         else
         {
            return BZ2_bzDecompress (&stream_);
         }
      }
            
      int BZLib::init ()
      {
         // default compression ratio of 4
         return (compress_ ? 
               BZ2_bzCompressInit (&stream_, 4, 0, 0)
             : BZ2_bzDecompressInit (&stream_, 0, 0));
      }
      int BZLib::done ()
      {
         return (compress_ ? BZ2_bzCompressEnd (&stream_)
               : BZ2_bzDecompressEnd (&stream_));
      }

      void BZLib::reset ()
      {
         curstate_ = SUPPLY_INBUF;
         if (stream_init_)
         {
            done ();
            stream_init_ = false;
         }
      }

      void BZLib::transform(const void * const in, size_t insize,
            void * out, size_t outsize, size_t * written,
            int * outstate, bool flushflag)
      {
         // After transform returns TRANSFORM_DONE, reset must be called
         // before trying to transform more data.
         ALWAYS_ASSERT(curstate_ != TRANSFORM_DONE);

         *written = 0;

         if (!stream_init_)
         {
            stream_.bzalloc = 0;
            stream_.bzfree = 0;
            stream_.opaque = 0;

            init ();
            stream_init_ = true;

            // bzlib is not const safe
            stream_.next_in = const_cast<char*>(static_cast<const char*>(in));
            stream_.avail_in = insize;
            stream_.next_out = static_cast<char*>(out);
            stream_.avail_out = outsize;
         }

         if (curstate_ == SUPPLY_INBUF)
         {
            stream_.next_in = const_cast<char*>(static_cast<const char*>(in));
            stream_.avail_in = insize;
         }
         else if (curstate_ == CONSUME_OUTBUF)
         {
            stream_.next_out = static_cast<char*>(out);
            stream_.avail_out = outsize;
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

            const int ret = process (flushflag);

            switch (ret)
            {
               case BZ_STREAM_END:
                  curstate_ = TRANSFORM_DONE;
                  *outstate = curstate_;
                  *written = outsize - stream_.avail_out;
                  check(done ());
                  stream_init_ = false;
                  return;
               case BZ_OK:
               case BZ_RUN_OK:
               case BZ_FLUSH_OK:
               case BZ_FINISH_OK:
                  break;
               default:
                  check (ret);
                  break;
            }
         } while (true);
      }

      state BZLib::getTransformState () const
      {
         return curstate_;
      }


      //=====================================================================
   }
}
