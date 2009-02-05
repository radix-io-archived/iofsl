#include "BMIBuffer.hh"
#include "iofwdutil/always_assert.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================


   BMIBuffer::~BMIBuffer ()
   {
      resize (0); 
   }

   BMIBuffer::BMIBuffer (BMIAddr addr, BMI::AllocType a)
      : addr_(addr), alloc_(a), ptr_(0), size_(0) 
   {
   }

   void BMIBuffer::resize (size_t newsize)
   {
      if (size_ != 0)
      {
         BMI::get().free (addr_, ptr_, size_, alloc_); 
         size_ = 0; 
         ptr_ = 0; 
      }
      if (!newsize)
         return; 
      ptr_ = BMI::get().alloc (addr_, newsize, alloc_); 
      ALWAYS_ASSERT (ptr_); 
      size_ = newsize; 
   }


//===========================================================================
   }
}
