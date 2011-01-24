#include "Timer.hh"

#include "iofwdevent/TimerResource.hh"

SERVICE_REGISTER(iofwd::Timer, timer);

namespace iofwd
{
   //========================================================================

   Timer::Timer (service::ServiceManager & m)
      : service::Service (m),
        timer_ (new iofwdevent::TimerResource ())
   {
      timer_->start ();
   }

   Timer::~Timer ()
   {
      timer_->stop ();
   }

   //========================================================================
}
