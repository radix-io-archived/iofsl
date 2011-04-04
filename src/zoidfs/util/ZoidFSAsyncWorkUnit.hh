#ifndef ZOIDFS_UTIL_ZOIDFSASYNCWORKUNIT_HH
#define ZOIDFS_UTIL_ZOIDFSASYNCWORKUNIT_HH

#include "ZoidFSAPI.hh"
#include "ZoidFSAsync.hh"
#include "iofwdutil/ThreadPool.hh"
#include "iofwdutil/LinkHelper.hh"
#include "zoidfs/zoidfs-proto.h"
#include "src/zoidfs/util/zoidfs-ops.hh"
#include "iofwdutil/mm/NBIOMemoryManager.hh"

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

namespace zoidfs
{
    namespace util
    {
        class zoidfs_async_write_op_key
        {
            public:
                zoidfs_async_write_op_key(zoidfs_handle_t handle,
                        uint64_t id) :
                    handle_(handle),
                    id_(id),
                    commit_id_(0)
                {
                }

                bool operator== (const zoidfs_async_write_op_key & rhs) const
                {
                    if(handle_ == rhs.handle_)
                    {
                        if(id_ == rhs.id_)
                        {
                            return true;
                        }
                    }
                    return false;
                }

                bool operator< (const zoidfs_async_write_op_key & rhs) const
                {
                    if(handle_ < rhs.handle_)
                    {
                        if(id_ < rhs.id_)
                        {
                            return true;
                        }
                    }
                    return false;
                }

                zoidfs_handle_t handle_;
                uint64_t id_;
                uint64_t commit_id_;
        };

        class ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        int type,
                        iofwdutil::ThreadPool & tp) :
                    cb_(cb),
                    ret_(ret),
                    api_(api),
                    type_(type),
                    tp_(tp)
                {
                }

                const iofwdevent::CBType cb_;
                int * ret_;
                ZoidFSAPI * api_;
                int type_;
                iofwdutil::ThreadPool & tp_;
        };

        class ZoidFSDefAsyncNullWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncNullWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp) : 
                    ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            zoidfs::ZOIDFS_PROTO_NULL, tp)
                {
                }
        };

        class ZoidFSDefAsyncNBWriteWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncNBWriteWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        size_t mem_count,
                        const void * mem_starts[],
                        const size_t mem_sizes[],
                        size_t file_count,
                        const zoidfs_file_ofs_t file_starts[],
                        const zoidfs_file_size_t file_sizes[],
                        zoidfs_op_hint_t * hint,
                        zoidfs_async_write_op_key * op_key = NULL,
                        iofwdutil::mm::NBIOMemoryAlloc * alloc = NULL) :

                    ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            zoidfs::ZOIDFS_PROTO_WRITE, tp),
                    handle_(handle), mem_count_(mem_count),
                    mem_starts_(mem_starts),
                    mem_sizes_(mem_sizes),
                    file_count_(file_count),
                    file_starts_(file_starts),
                    file_sizes_(file_sizes),
                    hint_(hint),
                    op_key_(op_key),
                    alloc_(alloc)
                {
                }

                boost::shared_ptr<const zoidfs_handle_t> handle_;
                size_t mem_count_;
                boost::shared_array<const void *> mem_starts_;
                boost::shared_array<const size_t> mem_sizes_;
                size_t file_count_;
                boost::shared_array<const zoidfs_file_ofs_t> file_starts_;
                boost::shared_array<const zoidfs_file_size_t> file_sizes_;
                zoidfs_op_hint_t * hint_;
                zoidfs_async_write_op_key * op_key_;
                iofwdutil::mm::NBIOMemoryAlloc * alloc_;
        };

        class ZoidFSDefAsyncWriteWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncWriteWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        size_t mem_count,
                        const void * mem_starts[],
                        const size_t mem_sizes[],
                        size_t file_count,
                        const zoidfs_file_ofs_t file_starts[],
                        const zoidfs_file_size_t file_sizes[],
                        zoidfs_op_hint_t * hint,
                        zoidfs_async_write_op_key * op_key = NULL,
                        iofwdutil::mm::NBIOMemoryAlloc * alloc = NULL) :

                    ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            zoidfs::ZOIDFS_PROTO_WRITE, tp),
                    handle_(handle), mem_count_(mem_count),
                    mem_starts_(mem_starts),
                    mem_sizes_(mem_sizes),
                    file_count_(file_count),
                    file_starts_(file_starts),
                    file_sizes_(file_sizes),
                    hint_(hint),
                    op_key_(op_key),
                    alloc_(alloc)
                {
                }

                const zoidfs_handle_t * handle_;
                size_t mem_count_;
                const void ** mem_starts_;
                const size_t * mem_sizes_;
                size_t file_count_;
                const zoidfs_file_ofs_t * file_starts_;
                const zoidfs_file_size_t * file_sizes_;
                zoidfs_op_hint_t * hint_;
                zoidfs_async_write_op_key * op_key_;
                iofwdutil::mm::NBIOMemoryAlloc * alloc_;
        };

        class ZoidFSDefAsyncGetattrWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncGetattrWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        zoidfs_attr_t * attr,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_GET_ATTR, tp),
                    handle_(handle),
                    attr_(attr),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                zoidfs_attr_t * attr_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncSetattrWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncSetattrWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        const zoidfs_sattr_t * sattr,
                        zoidfs_attr_t * attr,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_SET_ATTR, tp),
                    handle_(handle),
                    sattr_(sattr),
                    attr_(attr),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                const zoidfs_sattr_t * sattr_;
                zoidfs_attr_t * attr_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncLookupWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncLookupWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * parent_handle,
                        const char * component_name,
                        const char * full_path,
                        zoidfs_handle_t * handle,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_LOOKUP, tp),
                    parent_handle_(parent_handle),
                    component_name_(component_name),
                    full_path_(full_path),
                    handle_(handle),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * parent_handle_;
                const char * component_name_;
                const char * full_path_;
                zoidfs_handle_t * handle_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncReadlinkWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncReadlinkWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        char * buffer,
                        size_t buffer_length,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_READLINK, tp),
                    handle_(handle),
                    buffer_(buffer),
                    buffer_length_(buffer_length),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                char * buffer_;
                size_t buffer_length_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncReadWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncReadWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        size_t mem_count,
                        void * mem_starts[],
                        const size_t mem_sizes[],
                        size_t file_count,
                        const zoidfs_file_ofs_t file_starts[],
                        const zoidfs_file_size_t file_sizes[],
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_READ, tp),
                    handle_(handle),
                    mem_count_(mem_count),
                    mem_starts_(mem_starts),
                    mem_sizes_(mem_sizes),
                    file_count_(file_count),
                    file_starts_(file_starts),
                    file_sizes_(file_sizes),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                size_t mem_count_;
                void ** mem_starts_;
                const size_t * mem_sizes_;
                size_t file_count_;
                const zoidfs_file_ofs_t * file_starts_;
                const zoidfs_file_size_t * file_sizes_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncCommitWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncCommitWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_COMMIT, tp),
                    handle_(handle),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncCreateWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncCreateWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * parent_handle,
                        const char * component_name,
                        const char * full_path,
                        const zoidfs_sattr_t * sattr,
                        zoidfs_handle_t * handle,
                        int * created,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_CREATE, tp),
                    parent_handle_(parent_handle),
                    component_name_(component_name),
                    full_path_(full_path),
                    sattr_(sattr),
                    handle_(handle),
                    created_(created),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * parent_handle_;
                const char * component_name_;
                const char * full_path_;
                const zoidfs_sattr_t * sattr_;
                zoidfs_handle_t * handle_;
                int * created_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncRemoveWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncRemoveWorkUnit(const iofwdevent::CBType & cb,
                        int * ret, ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * parent_handle,
                        const char * component_name,
                        const char * full_path,
                        zoidfs_cache_hint_t * parent_hint,
                        zoidfs_op_hint_t * hint)
                    : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                            ZOIDFS_PROTO_REMOVE, tp),
                    parent_handle_(parent_handle),
                    component_name_(component_name),
                    full_path_(full_path),
                    parent_hint_(parent_hint),
                    hint_(hint)
                {
                }

                const zoidfs_handle_t * parent_handle_;
                const char * component_name_;
                const char * full_path_;
                zoidfs_cache_hint_t * parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncRenameWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncRenameWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * from_parent_handle,
                        const char * from_component_name,
                        const char * from_full_path,
                        const zoidfs_handle_t * to_parent_handle,
                        const char * to_component_name,
                        const char * to_full_path,
                        zoidfs_cache_hint_t * from_parent_hint,
                        zoidfs_cache_hint_t * to_parent_hint,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                        ZOIDFS_PROTO_RENAME, tp),
                from_parent_handle_(from_parent_handle),
                from_component_name_(from_component_name),
                from_full_path_(from_full_path),
                to_parent_handle_(to_parent_handle),
                to_component_name_(to_component_name),
                to_full_path_(to_full_path),
                from_parent_hint_(from_parent_hint),
                to_parent_hint_(to_parent_hint),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * from_parent_handle_;
                const char * from_component_name_;
                const char * from_full_path_;
                const zoidfs_handle_t * to_parent_handle_;
                const char * to_component_name_;
                const char * to_full_path_;
                zoidfs_cache_hint_t * from_parent_hint_;
                zoidfs_cache_hint_t * to_parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncLinkWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncLinkWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * from_parent_handle,
                        const char * from_component_name,
                        const char * from_full_path,
                        const zoidfs_handle_t * to_parent_handle,
                        const char * to_component_name,
                        const char * to_full_path,
                        zoidfs_cache_hint_t * from_parent_hint,
                        zoidfs_cache_hint_t * to_parent_hint,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api, ZOIDFS_PROTO_LINK,
                        tp),
                from_parent_handle_(from_parent_handle),
                from_component_name_(from_component_name),
                from_full_path_(from_full_path),
                to_parent_handle_(to_parent_handle),
                to_component_name_(to_component_name),
                to_full_path_(to_full_path),
                from_parent_hint_(from_parent_hint),
                to_parent_hint_(to_parent_hint),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * from_parent_handle_;
                const char * from_component_name_;
                const char * from_full_path_;
                const zoidfs_handle_t * to_parent_handle_;
                const char * to_component_name_;
                const char * to_full_path_;
                zoidfs_cache_hint_t * from_parent_hint_;
                zoidfs_cache_hint_t * to_parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncSymlinkWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncSymlinkWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * from_parent_handle,
                        const char * from_component_name,
                        const char * from_full_path,
                        const zoidfs_handle_t * to_parent_handle,
                        const char * to_component_name,
                        const char * to_full_path,
                        const zoidfs_sattr_t * sattr,
                        zoidfs_cache_hint_t * from_parent_hint,
                        zoidfs_cache_hint_t * to_parent_hint,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                        ZOIDFS_PROTO_SYMLINK, tp),
                from_parent_handle_(from_parent_handle),
                from_component_name_(from_component_name),
                from_full_path_(from_full_path),
                to_parent_handle_(to_parent_handle),
                to_component_name_(to_component_name),
                to_full_path_(to_full_path),
                sattr_(sattr),
                from_parent_hint_(from_parent_hint),
                to_parent_hint_(to_parent_hint),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * from_parent_handle_;
                const char * from_component_name_;
                const char * from_full_path_;
                const zoidfs_handle_t * to_parent_handle_;
                const char * to_component_name_;
                const char * to_full_path_;
                const zoidfs_sattr_t * sattr_;
                zoidfs_cache_hint_t * from_parent_hint_;
                zoidfs_cache_hint_t * to_parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncMkdirWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncMkdirWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * parent_handle,
                        const char * component_name,
                        const char * full_path,
                        const zoidfs_sattr_t * sattr,
                        zoidfs_cache_hint_t * parent_hint,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                        ZOIDFS_PROTO_MKDIR, tp),
                parent_handle_(parent_handle),
                component_name_(component_name),
                full_path_(full_path),
                sattr_(sattr),
                parent_hint_(parent_hint),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * parent_handle_;
                const char * component_name_;
                const char * full_path_;
                const zoidfs_sattr_t * sattr_;
                zoidfs_cache_hint_t * parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncReaddirWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncReaddirWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * parent_handle,
                        zoidfs_dirent_cookie_t cookie,
                        size_t * entry_count,
                        zoidfs_dirent_t * entries,
                        uint32_t flags,
                        zoidfs_cache_hint_t * parent_hint,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api, 
                        ZOIDFS_PROTO_READDIR, tp),
                parent_handle_(parent_handle),
                cookie_(cookie),
                entry_count_(entry_count),
                entries_(entries),
                flags_(flags),
                parent_hint_(parent_hint),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * parent_handle_;
                zoidfs_dirent_cookie_t cookie_;
                size_t * entry_count_;
                zoidfs_dirent_t * entries_;
                uint32_t flags_;
                zoidfs_cache_hint_t * parent_hint_;
                zoidfs_op_hint_t * hint_;
        };

        class ZoidFSDefAsyncResizeWorkUnit : public ZoidFSDefAsyncWorkUnit
        {
            public:
                ZoidFSDefAsyncResizeWorkUnit(const iofwdevent::CBType & cb,
                        int * ret,
                        ZoidFSAPI * api,
                        iofwdutil::ThreadPool & tp,
                        const zoidfs_handle_t * handle,
                        zoidfs_file_size_t size,
                        zoidfs_op_hint_t * hint)
                : ZoidFSDefAsyncWorkUnit(cb, ret, api,
                        ZOIDFS_PROTO_RESIZE, tp),
                handle_(handle),
                size_(size),
                hint_(hint)
                {
                }

                const zoidfs_handle_t * handle_;
                zoidfs_file_size_t size_;
                zoidfs_op_hint_t * hint_;
        };
    }
}

#endif
