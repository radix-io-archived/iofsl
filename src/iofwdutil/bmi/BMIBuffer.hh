#ifndef IOFWDUTIL_BMIBUFFER_HH
#define IOFWDUTIL_BMIBUFFER_HH

#include "bmi/BMI.hh"
#include "bmi/BMIAddr.hh"

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

         }

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
