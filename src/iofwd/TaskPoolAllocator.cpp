#include "iofwd/TaskPoolAllocator.hh"
#include <boost/pool/pool_alloc.hpp>
#include "zoidfs/zoidfs-proto.h"
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

namespace iofwd
{
    TaskPoolAllocator::TaskPoolAllocator()
    {
    }

    /* Force the release of all tasks allocated by the pool allocators */
    TaskPoolAllocator::~TaskPoolAllocator()
    {
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(NullTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(GetAttrTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(SetAttrTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(LookupTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(ReadLinkTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(CommitTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(CreateTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(RemoveTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(SymLinkTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(MkdirTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(ReadDirTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(ResizeTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(WriteTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(ReadTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(LinkTask)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(NotImplementedTask)>::release_memory();
    }

    /* allocate a task from the pool */
    Task * TaskPoolAllocator::allocate(int tt)
    {
        Task * t = NULL;
        switch (tt)
        {
            case zoidfs::ZOIDFS_PROTO_NULL:
            {
                t = null_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_GET_ATTR:
            {
                t = getattr_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SET_ATTR:
            {
                t = setattr_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LOOKUP:
            {
                t = lookup_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READLINK:
            {
                t = readlink_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_COMMIT:
            {
                t = commit_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_CREATE:
            {
                t = create_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_REMOVE:
            {
                t = remove_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RENAME:
            {
                t = rename_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SYMLINK:
            {
                t = symlink_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_MKDIR:
            {
                t = mkdir_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READDIR:
            {
                t = readdir_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RESIZE:
            {
                t = resize_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_WRITE:
            {
                t = write_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READ:
            {
                t = read_task_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LINK:
            {
                t = link_task_alloc_.allocate();
                break;
            }
            default:
            {
                t = not_implemented_task_alloc_.allocate();
                break;
            }
        };

        /* set the allocation ID... used in deallocate to determine what pool */
        t->setAllocID(tt);

        return t;
    }

    /* deallocate a task from the pool */
    void TaskPoolAllocator::deallocate(Task * t)
    {
        int tt = t->getAllocID();
        switch (tt)
        {
            case zoidfs::ZOIDFS_PROTO_NULL:
            {
                null_task_alloc_.deallocate(static_cast<NullTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_GET_ATTR:
            {
                getattr_task_alloc_.deallocate(static_cast<GetAttrTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SET_ATTR:
            {
                setattr_task_alloc_.deallocate(static_cast<SetAttrTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LOOKUP:
            {
                lookup_task_alloc_.deallocate(static_cast<LookupTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READLINK:
            {
                readlink_task_alloc_.deallocate(static_cast<ReadLinkTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_COMMIT:
            {
                commit_task_alloc_.deallocate(static_cast<CommitTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_CREATE:
            {
                create_task_alloc_.deallocate(static_cast<CreateTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_REMOVE:
            {
                remove_task_alloc_.deallocate(static_cast<RemoveTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RENAME:
            {
                rename_task_alloc_.deallocate(static_cast<RenameTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SYMLINK:
            {
                symlink_task_alloc_.deallocate(static_cast<SymLinkTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_MKDIR:
            {
                mkdir_task_alloc_.deallocate(static_cast<MkdirTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READDIR:
            {
                readdir_task_alloc_.deallocate(static_cast<ReadDirTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RESIZE:
            {
                resize_task_alloc_.deallocate(static_cast<ResizeTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_WRITE:
            {
                write_task_alloc_.deallocate(static_cast<WriteTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READ:
            {
                read_task_alloc_.deallocate(static_cast<ReadTask *>(t));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LINK:
            {
                link_task_alloc_.deallocate(static_cast<LinkTask *>(t));
                break;
            }
            default:
            {
                not_implemented_task_alloc_.deallocate(static_cast<NotImplementedTask *>(t));
                break;
            }
        };
    }
} /* namespace iofwd */
