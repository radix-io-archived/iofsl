#include "iofwdutil/assert.hh"
#include "ThreadTasks.hh"
#include "Request.hh"
#include "NotImplementedTask.hh"
#include "NullTask.hh"
#include "ReadTask.hh"
#include "zoidfs/zoidfs-proto.h"
#include "LookupTask.hh"
#include "TaskHelper.hh"

using namespace zoidfs;

namespace iofwd
{
//===========================================================================

Task * ThreadTasks::operator () (Request * req)
{
   ThreadTaskParam p (req, reschedule_, api_, bmi_); 
   switch (req->getOpID ())
   {
      case ZOIDFS_PROTO_NULL:
         return new NullTask (p); 
      case ZOIDFS_PROTO_LOOKUP:
         return new LookupTask (p); 
      case ZOIDFS_PROTO_READ:
         return new ReadTask (p); 
      default:
         return new NotImplementedTask (p); 
   }; 

   ALWAYS_ASSERT(false && "Should not get here!"); 
   return 0; 
}


//===========================================================================
}
