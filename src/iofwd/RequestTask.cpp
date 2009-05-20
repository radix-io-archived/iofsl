#include "RequestTask.hh"
#include "iofwdutil/assert.hh"

namespace iofwd
{
//===========================================================================

   RequestTask::~RequestTask ()
   {
   }

   void RequestTask::doWork ()
   {
      run (); 
      switch (getStatus())
      {
         case STATUS_RERUN:
            reschedule (); 
            break; 
         case STATUS_WAITING:
            // Do nothing, somebody else is going to call reschedule
            break; 
         case STATUS_DONE:
            // Do nothing, RequestHandler is going to cleanup
            break;
         default:
            ALWAYS_ASSERT(false && "Should not happen!"); 
      }
   }

//===========================================================================
}
