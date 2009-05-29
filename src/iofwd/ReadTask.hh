#ifndef IOFWD_READTASK_HH
#define IOFWD_READTASK_HH

#include "Task.hh"
#include "ReadRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class ReadTask : public Task, public TaskHelper<ReadRequest>
{
public:
   ReadTask (Request * req, boost::function<void (Task*)>
         & resched, zoidfs::ZoidFSAPI * api) 
      : Task (resched), TaskHelper<ReadRequest>(req, api)
   {
   }

   void run (); 

}; 

}


#endif
