#ifndef IOFWD_RESOURCES_HH
#define IOFWD_RESOURCES_HH

#include "iofwdevent/TimerResource.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdevent/ResourceWrapper.hh"

namespace iofwd
{
//===========================================================================

   /**
    * This class groups a number of resources so that they can easily be used
    * by other components in the system, by passing just a single reference
    * to this Resources struct.
    */
   struct Resources : public boost::noncopyable
   {
      public:
         Resources ()
            : timerwrap (&rtimer_), bmiwrap (&rbmi_), 
            tokenwrap (&rtoken_)
      {
      }

         iofwdevent::TimerResource          rtimer_;
         iofwdevent::BMIResource            rbmi_;
         iofwdevent::TokenResource          rtoken_;

      private:
         iofwdevent::ResourceWrapper        timerwrap;
         iofwdevent::ResourceWrapper        bmiwrap;
         iofwdevent::ResourceWrapper        tokenwrap;
   };

//===========================================================================
}

#endif
