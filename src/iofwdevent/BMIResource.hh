#ifndef IOFWDEVENT_BMIRESOURCE_HH
#define IOFWDEVENT_BMIRESOURCE_HH

extern "C" 
{
#include <bmi.h>
}

#include "Resource.hh"

namespace iofwdevent
{

   class BMIResource : public Resource
   {
   public:

      virtual void stop ();

      virtual void start ();

      BMIResource ();

      virtual ~BMIResource ();

   protected:
      bmi_context_id context_;
   };
}

#endif
