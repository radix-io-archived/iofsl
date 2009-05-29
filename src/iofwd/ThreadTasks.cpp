#include "iofwdutil/assert.hh"
#include "ThreadTasks.hh"
#include "Request.hh"
#include "NotImplementedTask.hh"
#include "NullTask.hh"
#include "ReadTask.hh"
#include "zoidfs/zoidfs-proto.h"
#include "LookupTask.hh"

using namespace zoidfs;

namespace iofwd
{
//===========================================================================

Task * ThreadTasks::operator () (Request * req)
{
   switch (req->getOpID ())
   {
      case ZOIDFS_PROTO_NULL:
         return new NullTask (req, reschedule_, api_); 
      case ZOIDFS_PROTO_LOOKUP:
         return new LookupTask (req, reschedule_, api_); 
      case ZOIDFS_PROTO_READ:
         return new ReadTask (req, reschedule_, api_); 
      default:
         return new NotImplementedTask (req, reschedule_, api_); 
   }; 

   ALWAYS_ASSERT(false && "Should not get here!"); 
   return 0; 
}


//===========================================================================
}
