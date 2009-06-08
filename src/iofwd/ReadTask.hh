#ifndef IOFWD_READTASK_HH
#define IOFWD_READTASK_HH

#include "Task.hh"
#include "ReadRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class ReadTask : public TaskHelper<ReadRequest>
{
public:
   ReadTask (ThreadTaskParam & p)
      : TaskHelper<ReadRequest>(p)
   {
   }

   void run (); 

}; 

}


#endif
