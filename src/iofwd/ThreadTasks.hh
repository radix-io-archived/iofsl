#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{
//===========================================================================

class Task;
class Request; 

/**
 * Task factory that allocates a dedicated thread for each request
 */
class ThreadTasks
{
public:

   ThreadTasks (boost::function<void (Task *)> & resched,
         zoidfs::ZoidFSAPI * api)
      : reschedule_(resched), api_(api)
   {
   }

   Task * operator () (Request * req); 

protected:
   boost::function<void (Task *)> reschedule_; 

   zoidfs::ZoidFSAPI * api_; 
}; 



//===========================================================================
}

#endif
