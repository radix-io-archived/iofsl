#include "iofwdutil/assert.hh"
#include "iofwd/tasksm/TaskSMFactory.hh"
#include "iofwd/Request.hh"
#include "zoidfs/zoidfs-proto.h"

#include "iofwd/tasksm/WriteTaskSM.hh"
#include "iofwd/tasksm/ReadTaskSM.hh"
#include "iofwd/tasksm/LookupTaskSM.hh"
#include "iofwd/tasksm/LinkTaskSM.hh"
#include "iofwd/tasksm/SymLinkTaskSM.hh"
#include "iofwd/tasksm/CreateTaskSM.hh"
#include "iofwd/tasksm/CommitTaskSM.hh"
#include "iofwd/tasksm/MkdirTaskSM.hh"
#include "iofwd/tasksm/NullTaskSM.hh"
#include "iofwd/tasksm/NotImplementedTaskSM.hh"
#include "iofwd/tasksm/ReadLinkTaskSM.hh"
#include "iofwd/tasksm/ReadDirTaskSM.hh"
#include "iofwd/tasksm/RemoveTaskSM.hh"
#include "iofwd/tasksm/RenameTaskSM.hh"
#include "iofwd/tasksm/ResizeTaskSM.hh"
#include "iofwd/tasksm/SetAttrTaskSM.hh"
#include "iofwd/tasksm/GetAttrTaskSM.hh"

using namespace zoidfs;

namespace iofwd
{
    namespace tasksm
    {

void TaskSMFactory::operator () (iofwd::Request * req)
{
    switch(req->getOpID())
    {
        case ZOIDFS_PROTO_LOOKUP:
            smm_.schedule(new LookupTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_LINK:
            smm_.schedule(new LinkTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_SYMLINK:
            smm_.schedule(new SymLinkTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_MKDIR:
            smm_.schedule(new MkdirTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_CREATE:
            smm_.schedule(new CreateTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_COMMIT:
            smm_.schedule(new CommitTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_NULL:
            smm_.schedule(new NullTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_READLINK:
            smm_.schedule(new ReadLinkTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_READDIR:
            smm_.schedule(new ReadDirTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_RENAME:
            smm_.schedule(new RenameTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_REMOVE:
            smm_.schedule(new RemoveTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_RESIZE:
            smm_.schedule(new ResizeTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_SET_ATTR:
            smm_.schedule(new SetAttrTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_GET_ATTR:
            smm_.schedule(new GetAttrTaskSM(smm_, api_, req));
            break;
        case ZOIDFS_PROTO_WRITE:
            smm_.schedule(new WriteTaskSM(smm_, sched_, req));
            break;
        case ZOIDFS_PROTO_READ:
            smm_.schedule(new ReadTaskSM(smm_, sched_, req));
            break;
        default:
            smm_.schedule(new NotImplementedTaskSM(smm_, api_, req));
            break;
   };
}

    }
}
