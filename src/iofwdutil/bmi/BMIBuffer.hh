#ifndef IOFWDUTIL_BMIBUFFER_HH
#define IOFWDUTIL_BMIBUFFER_HH

extern "C" 
{
#include "bmi.h"
}

#include "bmi/BMI.hh"
#include "bmi/BMIAddr.hh"
#include "iofwdutil/always_assert.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

   /**
    * AUto-resizing BMI buffer allocation
    */
   class BMIBuffer 
   {
      public:
         BMIBuffer (BMIAddr addr, BMI::AllocType a);

         void * get (size_t req) 
         {
            if (size_ >= req)
               return ptr_; 
            // need to resize
            resize (req); 
            ALWAYS_ASSERT (size_ >= req); 
            return ptr_; 
         }

         size_t size () const
         { return size_; } 

         void * get ()
         {
            return ptr_; 
         }


         bmi_buffer_type bmiType () const
         { return BMI_PRE_ALLOC; } 

         ~BMIBuffer (); 

      protected:
         void resize (size_t newsize); 

      protected:
         BMIAddr addr_; 
         BMI::AllocType alloc_; 
         void * ptr_; 
         size_t size_; 
   }; 

//===========================================================================
   }
}

#endif
