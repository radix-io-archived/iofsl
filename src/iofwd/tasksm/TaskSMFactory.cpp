#include "iofwdutil/assert.hh"
#include "iofwd/tasksm/TaskSMFactory.hh"
#include "iofwd/Request.hh"
#include "zoidfs/zoidfs-proto.h"

#include "iofwd/tasksm/WriteTaskSM.hh"

using namespace zoidfs;

namespace iofwd
{
    namespace tasksm
    {
//===========================================================================

void TaskSMFactory::operator () (iofwd::Request * req)
{
    switch(req->getOpID ())
    {
        case ZOIDFS_PROTO_WRITE:
            smm_.schedule(new WriteTaskSM(smm_, sched_, bpool_, req));
            return;
        default:
            return;
   };

   ALWAYS_ASSERT(false && "Should not get here!");
}
    }
//===========================================================================
}
