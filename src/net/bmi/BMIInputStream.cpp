#include "BMIInputStream.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

#include <boost/bind.hpp>

namespace net
{
   namespace bmi
   {
      //=====================================================================

      BMIInputStream::BMIInputStream (
            iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            BMI_addr_t addr,
            bmi_msg_tag_t tag,
            const BMIStreamHeader & h)
         : input_ (log, bmi, addr, tag, h),
           curbuf_ (0),
           curbuf_size_ (0),
           curbuf_used_ (0)
      {
      }

      BMIInputStream::BMIInputStream (
            iofwdutil::IOFWDLogSource & log,
            iofwdevent::BMIResource & bmi,
            const BMI_unexpected_info & info,
            bmi_msg_tag_t tag)
         : input_ (log, bmi, info, tag),
           curbuf_ (0),
           curbuf_size_ (0),
           curbuf_used_ (0)
      {
      }

      BMIInputStream::~BMIInputStream ()
      {
      }

      iofwdevent::Handle BMIInputStream::read (const void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t UNUSED(suggested))
      {
         if (curbuf_used_ < curbuf_size_)
         {
            *ptr = static_cast<const char *>(curbuf_) + curbuf_used_;
            *size = curbuf_size_ - curbuf_used_;
            curbuf_used_ = curbuf_size_;
            cb (iofwdevent::CBException ());
            return 0;
         }

         input_.read (&curbuf_, &curbuf_size_,
            boost::bind (&BMIInputStream::readCompleted, this, ptr, size, cb,
               _1));

         return 0;
      }

      void BMIInputStream::readCompleted (const void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e)
      {
         e.check ();

         *ptr = curbuf_;
         *size = curbuf_size_;
         curbuf_used_ = curbuf_size_;
         cb (e);
      }

      iofwdevent::Handle BMIInputStream::rewindInput (size_t size,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(size <= curbuf_used_);
         curbuf_used_ -= size;
         cb (iofwdevent::CBException ());
         return 0;
      }


      //=====================================================================
   }
}
