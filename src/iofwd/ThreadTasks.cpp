#include "iofwdutil/assert.hh"
#include "ThreadTasks.hh"
#include "Request.hh"
#include "NotImplementedTask.hh"

namespace iofwd
{
//===========================================================================

RequestTask * ThreadTasks::operator () (Request * req)
{
   switch (req->getOpID ())
   {
      return new NotImplementedTask (req, reschedule_); 
   }; 

   ALWAYS_ASSERT(false && "SHould not get here!"); 
   return 0; 
}


//===========================================================================
}
