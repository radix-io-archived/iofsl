#include "BMI.hh"
#include "BMIContext.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

      BMIContext::BMIContext ()
      {
         BMI::check(BMI_open_context (&context_)); 
      }

      BMIContext::~BMIContext ()
      {
      }

//===========================================================================
   }
}
