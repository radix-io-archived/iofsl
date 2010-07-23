#include <numeric>
#include <algorithm>

#include "IOWriteBuffer.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      size_t IOWriteBuffer::getWritten () const
      {
         return extbuf_used_;
      }

      void IOWriteBuffer::reset ()
      {
         extbuf_ = 0;
         extbuf_size_ = 0;
         extbuf_used_ = 0;

         intbuf_used_ = 0;
         intbuf_consumed_ = 0;
      }

      void * IOWriteBuffer::reserve (size_t bytes)
      {
         ASSERT(bytes <= maxaccess_);

         // for assertion checking; reserve and release should use same
         // segment size
         ASSERT(!lastsegsize_ || lastsegsize_ == bytes);

         // Reserve never fails for write buffers, so lastsegsize_ should be 0
         ASSERT(!lastsegsize_);

         lastsegsize_ = bytes;

         // if we have data in the internal buffer return that otherwise try to
         // get more.

         // There is no way the internal buffer can be in use when we call
         // reserve.
         ALWAYS_ASSERT(!intbuf_used_);

         // there is nothing in the internal buffer.
         // 2 possibilities:
         //    1) we have enough in the external buffer
         //        -> return pointer to external buffer
         //    2) we don't have enough in the external buffer
         //        -> copy what we can to internal buffer and request more
         const size_t extavail = extbuf_size_ - extbuf_used_;
         if (extavail >= bytes)
         {
            return extbuf_ + extbuf_used_;
         }
         else
         {
            // not enough room in external buffer;
            // allocate internal buffer for writes

            ASSERT(intbuf_used_ == 0);
            ASSERT(bytes <= maxaccess_);
            // for writes, we can just allocate the whole region in the
            // internal buffer; We'll have to deal with the external
            // buffer on release
            intbuf_used_ = bytes;
            return intbuf_.get();
         }
      }

      bool IOWriteBuffer::release (size_t bytes)
      {
         // For writes, we allow to release less then was reserved
         ASSERT(lastsegsize_ >= bytes);
         ASSERT(bytes <= maxaccess_);

         // we assume that release will be called with the same size or less
         // then the size that was reserved; As such, data is either
         // completely in the external or completely in the internal buffer.

         ASSERT(!intbuf_used_  || (intbuf_used_ >= bytes));
         ASSERT(intbuf_used_ || ((extbuf_size_ - extbuf_used_) >= bytes));

         if (intbuf_used_)
         {
            if (bytes < lastsegsize_)
            {
               // user decided to release less then was reserved. This is OK,but
               // update internal buffer size discarding the leftover

               // This check ensures the user doesn't change the release size
               // in the middel of a multi-stage release
               ALWAYS_ASSERT(intbuf_consumed_ < intbuf_used_);

               // Even stricter check: the size of the release can only be
               // changed on the /first/ call to release, so all the data is
               // still only in the internal buffer
               ALWAYS_ASSERT(intbuf_consumed_ == 0);
               intbuf_used_ = bytes;
               lastsegsize_ = bytes;
            }

            // In internal buffer, we need to copy back to external buffer
            const size_t released = std::min (extbuf_size_  - extbuf_used_,
                  bytes - intbuf_consumed_);

            // copy from internal to external buffer
            memcpy (extbuf_ + extbuf_used_, intbuf_.get() +
                  intbuf_consumed_, released);

            extbuf_used_ += released;
            intbuf_consumed_ += released;

            if (intbuf_consumed_ == intbuf_used_)
            {
               // if we get here, the release is complete (i.e. the user
               // won't loop and call release again before calling reserve)
               intbuf_consumed_ = intbuf_used_ = 0;
               lastsegsize_ = 0;

               // A return of bytes indicates we're done.
               return bytes;
            }
            else
            {
               // we don't clear lastsegsize; The user has to call update
               // and then call reserve again _with the same segment
               // size_
               // We return 0 to indicate that the release ran out of
               // buffer space. The user should call updateWrite and retry
               // the release
               extbuf_ = 0;
               return 0;
            }
         }
         else
         {
            // All data is in the external buffer; Nothing todo but to
            // increase the external buffer pointer.
            ALWAYS_ASSERT(extbuf_size_ - extbuf_used_ >= bytes);
            extbuf_used_ += bytes;
            lastsegsize_ = 0;
            return bytes;
         }
      }

      void IOWriteBuffer::update (void * ext, size_t extsize)
      {
         ALWAYS_ASSERT(!extbuf_);
         extbuf_ = static_cast<char*>(ext);
         extbuf_size_ = extsize;
         extbuf_used_ = 0;
      }

      //=====================================================================
   }
}
