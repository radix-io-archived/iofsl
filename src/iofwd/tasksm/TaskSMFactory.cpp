#include "iofwd/tasksm/TaskSMFactory.hh"
#include "iofwdutil/assert.hh"
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

namespace iofwd
{
    namespace tasksm
    {
       //====================================================================

       TaskSMFactory::TaskSMFactory(zoidfs::util::ZoidFSAsync * api,
             sm::SMManager & smm)
          :  shared_ (api, smm)
       {
       }

       TaskSMFactory::~TaskSMFactory()
       {
       }


       void TaskSMFactory::operator () (Request * req)
       {
          switch(req->getOpID())
          {
             case ZOIDFS_PROTO_LOOKUP:
                shared_.smm.schedule(new LookupTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_LINK:
                shared_.smm.schedule(new LinkTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_SYMLINK:
                shared_.smm.schedule(new SymLinkTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_MKDIR:
                shared_.smm.schedule(new MkdirTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_CREATE:
                shared_.smm.schedule(new CreateTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_COMMIT:
                shared_.smm.schedule(new CommitTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_NULL:
                shared_.smm.schedule(new NullTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_READLINK:
                shared_.smm.schedule(new ReadLinkTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_READDIR:
                shared_.smm.schedule(new ReadDirTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_RENAME:
                shared_.smm.schedule(new RenameTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_REMOVE:
                shared_.smm.schedule(new RemoveTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_RESIZE:
                shared_.smm.schedule(new ResizeTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_SET_ATTR:
                shared_.smm.schedule(new SetAttrTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_GET_ATTR:
                shared_.smm.schedule(new GetAttrTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_WRITE:
                shared_.smm.schedule(new WriteTaskSM(req, shared_));
                break;
             case ZOIDFS_PROTO_READ:
                shared_.smm.schedule(new ReadTaskSM(req, shared_));
                break;
             default:
                shared_.smm.schedule(new NotImplementedTaskSM(req, shared_));
                break;
          };
       }


       //====================================================================
    }
}
