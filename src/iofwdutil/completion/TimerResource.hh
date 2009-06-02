#ifndef IOFWDUTIL_COMPLETION_TIMERRESOURCE_HH
#define IOFWDUTIL_COMPLETION_TIMERRESOURCE_HH

#include "CompletionID.hh"
#include "ContextBase.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class TimerResource 
{
public:
   TimerResource (ContextBase & ctx); 

   CompletionID createTimer (unsigned int mstimeout); 

}; 


//===========================================================================
   }
}


#endif
