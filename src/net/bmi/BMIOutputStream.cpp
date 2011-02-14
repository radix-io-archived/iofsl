#include "BMIOutputStream.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"

#include <boost/bind.hpp>

namespace net
{
   namespace bmi
   {
      //=====================================================================

      BMIOutputStream::BMIOutputStream (
            iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            BMI_addr_t addr,
            bmi_msg_tag_t tag,
            bool incoming,
            const BMIStreamHeader & h)
         : output_ (log, bmi, addr, tag, incoming, h),
           curbuf_ (0),
           curbuf_size_ (0),
           curbuf_used_ (0)
      {
      }


      BMIOutputStream::~BMIOutputStream ()
      {
      }

      void BMIOutputStream::writeComplete (void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e)
      {
         e.check ();

         *ptr = curbuf_;
         *size = curbuf_size_;
         curbuf_used_ = curbuf_size_;
         cb (e);
      }



      iofwdevent::Handle BMIOutputStream::write (void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t suggested)
      {
         // If we have a buffer and there's free room first use it up
         if (curbuf_size_ && (curbuf_used_ < curbuf_size_))
         {
            *ptr = static_cast<char *>(curbuf_) + curbuf_used_;
            *size = curbuf_size_ - curbuf_used_;
            curbuf_used_ = curbuf_size_;
         }
         else
         {
            // Get a decent suggested size
            // @TODO: get some way of configuring this value
            if (!suggested)
               suggested = 1*1024*1024;

            if (curbuf_)
            {
               // we need to send the current buffer first
               output_.flush (curbuf_used_,
                     boost::bind (&BMIOutputStream::flushAndAlloc,
                        this, ptr, size, suggested, cb, _1));
               return 0;
            }

            // Just allocate
            doAlloc (ptr, size, suggested);
         }

         cb (iofwdevent::CBException ());

         return 0;
      }


      void BMIOutputStream::flushAndAlloc (void ** ptr, size_t * size,
            size_t suggested, const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e)
      {
         doAlloc (ptr, size, suggested);
         cb (e);
      }


      void BMIOutputStream::doAlloc (void ** ptr, size_t * size, size_t
            suggested)
      {
         output_.allocWrite (ptr, size, suggested);

         curbuf_size_ = *size;
         curbuf_ = *ptr;
         curbuf_used_ = curbuf_size_;
      }

      iofwdevent::Handle BMIOutputStream::rewindOutput (size_t size,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(curbuf_used_ == curbuf_size_ && curbuf_used_);
         ALWAYS_ASSERT(size <= curbuf_used_);
         curbuf_used_ -= size;
         cb (iofwdevent::CBException ());
         return 0;
      }

      iofwdevent::Handle BMIOutputStream::flush (const iofwdevent::CBType & cb)
      {
         const size_t used = curbuf_used_;
         curbuf_ = 0;
         curbuf_size_ = 0;
         curbuf_used_ = 0;

         // reset before calling callback in case the callback calls write
         // again
         output_.flush (used, cb);
         return 0;
      }


      //=====================================================================
   }
}
