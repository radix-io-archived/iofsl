#include "BMIResource.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

      BMIRersource::BMIResource (ContextBase & ctx)
         : ctx_(ctx)
      {
         checkBMI (BMI_open_context (&bmictx_)); 
      }

      BMIResource::~BMIResource ()
      {
         checkBMI (BMI_close_context(&bmictx_)); 
      }

//===========================================================================
   }
}
