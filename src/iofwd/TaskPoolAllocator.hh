#ifndef __IOFWD_TASK_POOL_ALLOCATOR_HH__
#define __IOFWD_TASK_POOL_ALLOCATOR_HH__

#include <boost/pool/pool_alloc.hpp>

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

#include "iofwdutil/Singleton.hh"

namespace iofwd
{

/* Task allocator using the boost fast pool allocator for each iofwd task type */
class TaskPoolAllocator : public iofwdutil::Singleton< TaskPoolAllocator >
{
    public:
        TaskPoolAllocator();
        ~TaskPoolAllocator();

        /* allocate a task from the pool */
        Task * allocate(int tt);

        /* deallocate a task from the pool */
        void deallocate(Task * t);

    protected:

        /* allocators */
        boost::fast_pool_allocator< NullTask > null_task_alloc_;
        boost::fast_pool_allocator< GetAttrTask > getattr_task_alloc_;
        boost::fast_pool_allocator< SetAttrTask > setattr_task_alloc_;
        boost::fast_pool_allocator< LookupTask > lookup_task_alloc_;
        boost::fast_pool_allocator< ReadLinkTask > readlink_task_alloc_;
        boost::fast_pool_allocator< CommitTask > commit_task_alloc_;
        boost::fast_pool_allocator< CreateTask > create_task_alloc_;
        boost::fast_pool_allocator< RemoveTask > remove_task_alloc_;
        boost::fast_pool_allocator< RenameTask > rename_task_alloc_;
        boost::fast_pool_allocator< SymLinkTask > symlink_task_alloc_;
        boost::fast_pool_allocator< MkdirTask > mkdir_task_alloc_;
        boost::fast_pool_allocator< ReadDirTask > readdir_task_alloc_;
        boost::fast_pool_allocator< ResizeTask > resize_task_alloc_;
        boost::fast_pool_allocator< WriteTask > write_task_alloc_;
        boost::fast_pool_allocator< ReadTask > read_task_alloc_;
        boost::fast_pool_allocator< LinkTask > link_task_alloc_;
        boost::fast_pool_allocator< NotImplementedTask > not_implemented_task_alloc_;
        
};

} /* namespace iofwd */
#endif
