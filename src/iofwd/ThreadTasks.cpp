#include "iofwdutil/assert.hh"
#include "ThreadTasks.hh"
#include "Request.hh"
#include "NotImplementedTask.hh"
#include "NullTask.hh"

namespace iofwd
{
//===========================================================================

RequestTask * ThreadTasks::operator () (Request * req)
{
   switch (req->getOpID ())
   {
      case 0:
         return new NullTask (req, reschedule_); 
      default:
         return new NotImplementedTask (req, reschedule_); 
   }; 

   ALWAYS_ASSERT(false && "Should not get here!"); 
   return 0; 
}


//===========================================================================
}
