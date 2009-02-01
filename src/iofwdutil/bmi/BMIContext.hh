#ifndef IOFWDUTIL_BMI_BMICONTEXT_HH
#define IOFWDUTIL_BMI_BMICONTEXT_HH

extern "C" 
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIContext
{
protected:
   BMIContext (); 

   bmi_context_id getID () const
   { 
      return context_; 
   }
public:
   ~BMIContext (); 

protected:
   bmi_context_id context_; 
}; 


//===========================================================================
   }
}

#endif
