#include "BMIResource.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

      BMIResource::BMIResource (ContextBase & ctx)
         : ctx_(ctx)
      {
         checkBMI (BMI_open_context (&bmictx_)); 
      }

      BMIResource::~BMIResource ()
      {
         BMI_close_context(bmictx_); 
      }

      int BMIResource::handleBMIError (int ret) const
      {
         ASSERT(ret >=0 && "bmi error"); 
         return ret; 
      }

//===========================================================================
   }
}
