#ifndef __IOFWD_REQUEST_POOL_ALLOCATOR_HH__
#define __IOFWD_REQUEST_POOL_ALLOCATOR_HH__

#include <boost/pool/pool_alloc.hpp>

#include "iofwd/Request.hh"

#include "iofwd/frontend/IOFWDNullRequest.hh"
#include "iofwd/frontend/IOFWDGetAttrRequest.hh"
#include "iofwd/frontend/IOFWDSetAttrRequest.hh"
#include "iofwd/frontend/IOFWDLookupRequest.hh"
#include "iofwd/frontend/IOFWDReadLinkRequest.hh"
#include "iofwd/frontend/IOFWDCommitRequest.hh"
#include "iofwd/frontend/IOFWDCreateRequest.hh"
#include "iofwd/frontend/IOFWDRemoveRequest.hh"
#include "iofwd/frontend/IOFWDRenameRequest.hh"
#include "iofwd/frontend/IOFWDSymLinkRequest.hh"
#include "iofwd/frontend/IOFWDMkdirRequest.hh"
#include "iofwd/frontend/IOFWDReadDirRequest.hh"
#include "iofwd/frontend/IOFWDResizeRequest.hh"
#include "iofwd/frontend/IOFWDWriteRequest.hh"
#include "iofwd/frontend/IOFWDReadRequest.hh"
#include "iofwd/frontend/IOFWDLinkRequest.hh"
#include "iofwd/frontend/IOFWDNotImplementedRequest.hh"

#include "iofwdutil/Singleton.hh"

namespace iofwd
{

/* Request allocator using the boost fast pool allocator for each iofwd request type */
class RequestPoolAllocator : public iofwdutil::Singleton< RequestPoolAllocator >
{
    public:
        RequestPoolAllocator();
        ~RequestPoolAllocator();

        /* allocate a request from the pool */
        Request * allocate(int rt);

        /* deallocate a request from the pool */
        void deallocate(Request * r);

    protected:

        /* allocators */
        boost::fast_pool_allocator< iofwd::frontend::IOFWDNullRequest > null_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDGetAttrRequest > getattr_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDSetAttrRequest > setattr_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDLookupRequest > lookup_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDReadLinkRequest > readlink_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDCommitRequest > commit_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDCreateRequest > create_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDRemoveRequest > remove_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDRenameRequest > rename_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDSymLinkRequest > symlink_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDMkdirRequest > mkdir_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDReadDirRequest > readdir_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDResizeRequest > resize_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDWriteRequest > write_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDReadRequest > read_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDLinkRequest > link_request_alloc_;
        boost::fast_pool_allocator< iofwd::frontend::IOFWDNotImplementedRequest > not_implemented_request_alloc_;
        
};

} /* namespace iofwd */
#endif
