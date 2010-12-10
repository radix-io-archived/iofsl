#include <stdio.h>
#include <numeric>
#include <algorithm>

#include "IOReadBuffer.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      bool IOReadBuffer::empty () const
      { return getAvail () == 0; }

      size_t IOReadBuffer::getAvail () const
      {
         // size if sum of data amount in internal buffer and what's available
         // in the external buffer
         return intbuf_used_ + (extbuf_size_ - extbuf_used_);
      }

      void IOReadBuffer::reset ()
      {
         extbuf_ = 0;
         extbuf_size_ = 0;
         extbuf_used_ = 0;
         intbuf_used_ = 0;
         lastsegsize_ = 0;
      }

      const void * IOReadBuffer::reserve (size_t bytes)
      {
         ASSERT(bytes <= maxaccess_);

         // for assertion checking; reserve and release should use same
         // segment size

         // See note in CopyTransform.cpp why this cannot be required.
         //ASSERT(!lastsegsize_ || lastsegsize_ == bytes);

         lastsegsize_ = bytes;

         // if we have data in the internal buffer return that otherwise try to
         // get more.

         if (intbuf_used_)
         {
            if (intbuf_used_ >= bytes)
               return intbuf_.get();

            // we have some bytes, but enough
            const size_t needmore = bytes - intbuf_used_;
            const size_t havemore = std::min(extbuf_size_ - extbuf_used_,
                  needmore);
            ALWAYS_ASSERT(intbuf_used_ + havemore <= maxaccess_);

            // for reads we need to copy to the internal buffer;
            // we consume the extbuf when copying the data;
            // For writes, we just use internal buffer space and consume
            // the external buffer on release
            memcpy (intbuf_.get() + intbuf_used_, extbuf_ + extbuf_used_,
                  havemore);
            extbuf_used_ += havemore;

            intbuf_used_ += havemore;

            if (intbuf_used_ >= bytes)
            {
               return intbuf_.get();
            }
            else
            {
               extbuf_ = 0;
               // we don't have enough space (API violation) or we don't have
               // enough bytes in the external buffer
               return 0;
            }
         }
         else
         {
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
               // not enough room in external buffer; copy to internal buffer
               // for reads, allocate internal buffer for writes

               ASSERT(intbuf_used_ == 0);
               ASSERT(bytes <= maxaccess_);

               memcpy (intbuf_.get() + intbuf_used_, extbuf_ + extbuf_used_,
                     extavail);

               intbuf_used_ += extavail;

               // Also update the external buffer (a release from the internal
               // buffer won't advance extbuf_used_)
               extbuf_used_ += extavail;

               ALWAYS_ASSERT(intbuf_used_ <= intbuf_size_);

               extbuf_ = 0;
               return 0;
            }
         }
      }

      void IOReadBuffer::release (size_t bytes)
      {
         ASSERT(lastsegsize_ == bytes);
         ASSERT(bytes <= maxaccess_);

         // we assume that release will be called with the same size as
         // reserve; As such, data is either completely in the external or
         // completely in the internal buffer.
         ASSERT(!intbuf_used_  || (intbuf_used_ >= bytes));
         ASSERT(intbuf_used_ || ((extbuf_size_ - extbuf_used_) >= bytes));
            
         lastsegsize_ = 0;

         if (intbuf_used_)
         {
            // in read mode, we can always drop the internal buffer, and
            // given the assumption above bytes should == intbuf_used_
            ALWAYS_ASSERT(intbuf_used_ == bytes);
            intbuf_used_ = 0;

            // no need to update extbuf_used_ since we already did that
            // when we copied data into the internal buffer
         }
         else
         {
            // All data is in the external buffer; Nothing todo
            ALWAYS_ASSERT(extbuf_size_ - extbuf_used_ >= bytes);
            extbuf_used_ += bytes;
            lastsegsize_ = 0;
         }
      }

      void IOReadBuffer::update (const void * ext, size_t extsize)
      {
         ALWAYS_ASSERT(!extbuf_);
         extbuf_ = static_cast<const char*>(ext);
         extbuf_size_ = extsize;
         extbuf_used_ = 0;
      }

      //=====================================================================
   }
}
