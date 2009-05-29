
#ifndef IOFWD_LOOKUPTASK_HH
#define IOFWD_LOOKUPTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "LookupRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class LookupTask : public Task, public TaskHelper<LookupRequest>
{
public:
   LookupTask (Request * req, boost::function<void (Task*)>
         & resched, zoidfs::ZoidFSAPI * api) 
      : Task (resched), TaskHelper<LookupRequest>(req, api)
   {
   }

   void run ();

}; 

}


#endif
