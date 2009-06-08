
#ifndef IOFWD_LOOKUPTASK_HH
#define IOFWD_LOOKUPTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "LookupRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class LookupTask : public TaskHelper<LookupRequest>
{
public:
   LookupTask (ThreadTaskParam & p)
      : TaskHelper<LookupRequest>(p)
   {
   }

   void run ();

}; 

}


#endif
