#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>

namespace iofwd
{
//===========================================================================

class RequestTask;
class Request; 

/**
 * Task factory that allocates a dedicated thread for each request
 */
class ThreadTasks
{
public:

   ThreadTasks (boost::function<void (RequestTask *)> & resched)
      : reschedule_(resched)
   {
   }

   RequestTask * operator () (Request * req); 

protected:
   boost::function<void (RequestTask *)> reschedule_; 
}; 



//===========================================================================
}

#endif
