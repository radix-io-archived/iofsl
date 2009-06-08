#ifndef TIMERCOMPLETIONID_HH
#define TIMERCOMPLETIONID_HH

#include "CompletionID.hh"
#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace iofwdutil
{
   namespace completion
   {

class TimerResource; 

class TimerCompletionID : public CompletionID 
{
public:

   TimerCompletionID ()
      : reserved_(false)
   {
   }

protected:
   friend class TimerResource;

protected:
   boost::posix_time::ptime alarm_; 

   bool reserved_; 

};


   }
}

#endif
