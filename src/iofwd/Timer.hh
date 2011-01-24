#ifndef IOFWD_TIMER_HH
#define IOFWD_TIMER_HH

#include "iofwd/service/Service.hh"

#include <boost/scoped_ptr.hpp>

namespace iofwdevent
{
   class TimerResource;
}

namespace iofwd
{
   //========================================================================

   /**
    * Timer service.
    */
   class Timer : public service::Service
   {
      public:
         Timer (service::ServiceManager & m);

         virtual ~Timer ();

         iofwdevent::TimerResource & get ()
         { return *timer_.get(); }

      protected:
         boost::scoped_ptr<iofwdevent::TimerResource> timer_;
   };

   //========================================================================
}

#endif
