#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDMANAGER_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDMANAGER_HH

#include "iofwdutil/Singleton.hh"
#include "iofwdutil/ThreadPool.hh"

#include "zoidfs/zoidfs.h"

#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"

#include <boost/thread.hpp>

namespace iofwd
{
    namespace extraservice
    {
        class AtomicAppendManager : public iofwdutil::Singleton<
                                    AtomicAppendManager >
        {
            public:
                AtomicAppendManager() :
                    aarpc_tp_(iofwdutil::ThreadPool::instance()),
                    rpc_(iofwd::service::ServiceManager::instance().loadService<
                            iofwd::extraservice::AtomicAppendClientRPC>("aarpcclient"))
                {
                }

                ~AtomicAppendManager()
                {
                }

            protected:

                enum AARPCWorkUnitType
                {
                    AA_CREATE_OFFSET_WU = 0,
                    AA_DELETE_OFFSET_WU,
                    AA_GETNEXT_OFFSET_WU,
                    AA_UNKNOWN_WU,
                };

                iofwdutil::ThreadPool & aarpc_tp_;
                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc_;

                /* base type for the atomic append work units */
                class AtomicAppendWorkUnit
                {
                    public:
                        AtomicAppendWorkUnit(const iofwdevent::CBType & cb,
                             int type,
                             iofwdutil::ThreadPool & tp,
                             boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc) :
                            cb_(cb),
                            type_(type),
                            tp_(tp),
                            rpc_(rpc)
                        {
                        }

                        const iofwdevent::CBType cb_;
                        int type_;
                        iofwdutil::ThreadPool & tp_;
                        boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc_;
                };

                /* Create offset work unit */
                class AtomicAppendCreateOffsetWorkUnit : public AtomicAppendWorkUnit
                {
                    public:
                        AtomicAppendCreateOffsetWorkUnit(const iofwdevent::CBType & cb, 
                                iofwdutil::ThreadPool & tp,
                                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc,
                                zoidfs::zoidfs_handle_t * h,
                                zoidfs::zoidfs_file_size_t * o) :
                            AtomicAppendWorkUnit(cb, AA_CREATE_OFFSET_WU, tp,
                                    rpc),
                            handle(h),
                            offset(o),
                            retcode(0)
                        {
                        }

                        zoidfs::zoidfs_handle_t * handle;
                        zoidfs::zoidfs_file_size_t * offset;
                        uint64_t retcode;
                };
       
                /* Delete offset work unit */
                class AtomicAppendDeleteOffsetWorkUnit : public AtomicAppendWorkUnit
                {
                    public:
                        AtomicAppendDeleteOffsetWorkUnit(const iofwdevent::CBType & cb, 
                                iofwdutil::ThreadPool & tp,
                                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc,
                                zoidfs::zoidfs_handle_t * h,
                                zoidfs::zoidfs_file_size_t * o) :
                            AtomicAppendWorkUnit(cb, AA_DELETE_OFFSET_WU, tp, rpc),
                            handle(h),
                            offset(o),
                            retcode(0)
                        {
                        }

                        zoidfs::zoidfs_handle_t * handle;
                        zoidfs::zoidfs_file_size_t * offset;
                        uint64_t retcode;
                };
       
                /* Get next offset work unit */
                class AtomicAppendGetNextOffsetWorkUnit :
                    public AtomicAppendWorkUnit
                {
                    public:
                        AtomicAppendGetNextOffsetWorkUnit(
                                const iofwdevent::CBType & cb, 
                                iofwdutil::ThreadPool & tp,
                                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc,
                                zoidfs::zoidfs_handle_t * h,
                                zoidfs::zoidfs_file_size_t i,
                                zoidfs::zoidfs_file_size_t * o) :
                            AtomicAppendWorkUnit(cb, AA_GETNEXT_OFFSET_WU, tp, rpc),
                            handle(h),
                            incsize(i),
                            offset(o),
                            retcode(0)
                        {
                        }

                        zoidfs::zoidfs_handle_t * handle;
                        zoidfs::zoidfs_file_size_t incsize;
                        zoidfs::zoidfs_file_size_t * offset;
                        uint64_t retcode;
                };
       
                /* handle work unit: submit work unit to thread pool or spawn a thread
                * */ 
                void submitWorkUnit(const boost::function<void (void)> & wu_,
                        iofwdutil::ThreadPool::TPPrio prio, bool use_thread_pool=true)
                {
                    /* if use thread pool option, submit wu onto thread pool */
                    if(use_thread_pool)
                    {
                        aarpc_tp_.submitWorkUnit(wu_, prio);
                    }
                    /* else spawn and detach a thread */
                    else
                    {
                        boost::thread t(wu_);
                    }
                }

                /* run the AARPC work unit based on the given args */
                static void runAARPCWorkUnit(AtomicAppendWorkUnit * aawu);

            public:
                void issueCreateOffsetRPC(const iofwdevent::CBType & cb,
                        zoidfs::zoidfs_handle_t * handle,
                        zoidfs::zoidfs_file_size_t * offset);

                void issueDeleteOffsetRPC(const iofwdevent::CBType & cb,
                        zoidfs::zoidfs_handle_t * handle,
                        zoidfs::zoidfs_file_size_t * offset);

                void issueGetNextOffsetRPC(const iofwdevent::CBType & cb,
                        zoidfs::zoidfs_handle_t * handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_size_t * offset);

        };
    }
}

#endif
