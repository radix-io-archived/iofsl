#include "iofwdutil/assert.hh"
#include "ThreadTasks.hh"
#include "Request.hh"
#include "zoidfs/zoidfs-proto.h"
#include "TaskHelper.hh"

#include "NullTask.hh"
#include "GetAttrTask.hh"
#include "SetAttrTask.hh"
#include "LookupTask.hh"
#include "ReadLinkTask.hh"
#include "CommitTask.hh"
#include "CreateTask.hh"
#include "RemoveTask.hh"
#include "RenameTask.hh"
#include "SymLinkTask.hh"
#include "MkdirTask.hh"
#include "ReadDirTask.hh"
#include "ResizeTask.hh"
#include "WriteTask.hh"
#include "ReadTask.hh"
#include "LinkTask.hh"
#include "NotImplementedTask.hh"

#include "iofwd/TaskPoolAllocator.hh"

using namespace zoidfs;

namespace iofwd
{
//===========================================================================

Task * ThreadTasks::operator () (Request * req)
{
   ThreadTaskParam p (req, reschedule_, api_, async_api_, sched_, bpool_, bmi_);
#ifndef USE_TASK_POOL_ALLOCATOR
   switch (req->getOpID ())
   {
      case ZOIDFS_PROTO_NULL:
         return new NullTask (p);
      case ZOIDFS_PROTO_GET_ATTR:
         return new GetAttrTask (p);
      case ZOIDFS_PROTO_SET_ATTR:
         return new SetAttrTask (p);
      case ZOIDFS_PROTO_LOOKUP:
         return new LookupTask (p);
      case ZOIDFS_PROTO_READLINK:
         return new ReadLinkTask (p);
      case ZOIDFS_PROTO_COMMIT:
         return new CommitTask (p);
      case ZOIDFS_PROTO_CREATE:
         return new CreateTask (p);
      case ZOIDFS_PROTO_REMOVE:
         return new RemoveTask (p);
      case ZOIDFS_PROTO_RENAME:
         return new RenameTask (p);
      case ZOIDFS_PROTO_SYMLINK:
         return new SymLinkTask (p);
      case ZOIDFS_PROTO_MKDIR:
         return new MkdirTask (p);
      case ZOIDFS_PROTO_READDIR:
         return new ReadDirTask (p);
      case ZOIDFS_PROTO_RESIZE:
         return new ResizeTask (p);
      case ZOIDFS_PROTO_WRITE:
      {
         /* try to get a task from the pool */
         Task * t = tpool_->get(zoidfs::ZOIDFS_PROTO_WRITE);
         if(t)
         {
            /* set the new task parameters */
            static_cast<TaskPoolHelper<WriteRequest> *>(t)->resetTaskHelper(p);
            return t;
         }
         /* else. allocate a new task */
         else
         {
            return new WriteTask (p);
         }
      }
      case ZOIDFS_PROTO_READ:
         return new ReadTask (p);
      case ZOIDFS_PROTO_LINK:
         return new LinkTask (p);
      default:
         return new NotImplementedTask (p);
   };
#else
   /* get the mem location from the pool and then use placement new to create the object */
   void * task_mem_loc = iofwd::TaskPoolAllocator::instance().allocate(req->getOpID());
   switch (req->getOpID ())
   {
      case ZOIDFS_PROTO_NULL:
         return new (task_mem_loc) NullTask (p);
      case ZOIDFS_PROTO_GET_ATTR:
         return new (task_mem_loc) GetAttrTask (p);
      case ZOIDFS_PROTO_SET_ATTR:
         return new (task_mem_loc) SetAttrTask (p);
      case ZOIDFS_PROTO_LOOKUP:
         return new (task_mem_loc) LookupTask (p);
      case ZOIDFS_PROTO_READLINK:
         return new (task_mem_loc) ReadLinkTask (p);
      case ZOIDFS_PROTO_COMMIT:
         return new (task_mem_loc) CommitTask (p);
      case ZOIDFS_PROTO_CREATE:
         return new (task_mem_loc) CreateTask (p);
      case ZOIDFS_PROTO_REMOVE:
         return new (task_mem_loc) RemoveTask (p);
      case ZOIDFS_PROTO_RENAME:
         return new (task_mem_loc) RenameTask (p);
      case ZOIDFS_PROTO_SYMLINK:
         return new (task_mem_loc) SymLinkTask (p);
      case ZOIDFS_PROTO_MKDIR:
         return new (task_mem_loc) MkdirTask (p);
      case ZOIDFS_PROTO_READDIR:
         return new (task_mem_loc) ReadDirTask (p);
      case ZOIDFS_PROTO_RESIZE:
         return new (task_mem_loc) ResizeTask (p);
      case ZOIDFS_PROTO_WRITE:
         return new (task_mem_loc) WriteTask (p);
      case ZOIDFS_PROTO_READ:
         return new (task_mem_loc) ReadTask (p);
      case ZOIDFS_PROTO_LINK:
         return new (task_mem_loc) LinkTask (p);
      default:
         return new (task_mem_loc) NotImplementedTask (p);
   };
#endif

   ALWAYS_ASSERT(false && "Should not get here!");
   return 0;
}

//===========================================================================
}
