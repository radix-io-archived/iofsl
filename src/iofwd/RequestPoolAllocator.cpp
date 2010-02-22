#include "iofwd/RequestPoolAllocator.hh"
#include <boost/pool/pool_alloc.hpp>
#include "zoidfs/zoidfs-proto.h"

namespace iofwd
{
    RequestPoolAllocator::RequestPoolAllocator()
    {
    }

    /* Force the release of all requests allocated by the pool allocators */
    RequestPoolAllocator::~RequestPoolAllocator()
    {
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDNullRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDGetAttrRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDSetAttrRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDLookupRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDReadLinkRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDCommitRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDCreateRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDRemoveRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDSymLinkRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDMkdirRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDReadDirRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDResizeRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDWriteRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDReadRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDLinkRequest)>::release_memory();
        boost::singleton_pool<boost::pool_allocator_tag, sizeof(iofwd::frontend::IOFWDNotImplementedRequest)>::release_memory();
    }

    /* allocate a request from the pool */
    Request * RequestPoolAllocator::allocate(int rt)
    {
        Request * r = NULL;
        switch (rt)
        {
            case zoidfs::ZOIDFS_PROTO_NULL:
            {
                r = null_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_GET_ATTR:
            {
                r = getattr_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SET_ATTR:
            {
                r = setattr_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LOOKUP:
            {
                r = lookup_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READLINK:
            {
                r = readlink_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_COMMIT:
            {
                r = commit_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_CREATE:
            {
                r = create_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_REMOVE:
            {
                r = remove_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RENAME:
            {
                r = rename_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SYMLINK:
            {
                r = symlink_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_MKDIR:
            {
                r = mkdir_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READDIR:
            {
                r = readdir_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RESIZE:
            {
                r = resize_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_WRITE:
            {
                r = write_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READ:
            {
                r = read_request_alloc_.allocate();
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LINK:
            {
                r = link_request_alloc_.allocate();
                break;
            }
            default:
            {
                r = not_implemented_request_alloc_.allocate();
                break;
            }
        };

        /* set the allocation ID... used in deallocate to determine what pool */
        r->setAllocID(rt);

        return r;
    }

    /* deallocate a request from the pool */
    void RequestPoolAllocator::deallocate(Request * r)
    {
        int rt = r->getAllocID();
        switch (rt)
        {
            case zoidfs::ZOIDFS_PROTO_NULL:
            {
                null_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDNullRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_GET_ATTR:
            {
                getattr_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDGetAttrRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SET_ATTR:
            {
                setattr_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDSetAttrRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LOOKUP:
            {
                lookup_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDLookupRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READLINK:
            {
                readlink_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDReadLinkRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_COMMIT:
            {
                commit_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDCommitRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_CREATE:
            {
                create_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDCreateRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_REMOVE:
            {
                remove_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDRemoveRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RENAME:
            {
                rename_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDRenameRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_SYMLINK:
            {
                symlink_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDSymLinkRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_MKDIR:
            {
                mkdir_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDMkdirRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READDIR:
            {
                readdir_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDReadDirRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_RESIZE:
            {
                resize_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDResizeRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_WRITE:
            {
                write_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDWriteRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_READ:
            {
                read_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDReadRequest *>(r));
                break;
            }
            case zoidfs::ZOIDFS_PROTO_LINK:
            {
                link_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDLinkRequest *>(r));
                break;
            }
            default:
            {
                not_implemented_request_alloc_.deallocate(static_cast<iofwd::frontend::IOFWDNotImplementedRequest *>(r));
                break;
            }
        };
    }
} /* namespace iofwd */
