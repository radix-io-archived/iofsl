#include "BMIResource.hh"

namespace iofwdevent
{

   void BMIResource::stop ()
   {
   }

   void BMIResource::start ()
   {
   }

   BMIResource::~BMIResource ()
   {
      BMI_close_context (context_);
   }

   BMIResource::BMIResource ()
   {
      BMI_open_context (&context_);
   }

}
