#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDMANAGER_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDMANAGER_HH

#include "iofwdutil/Singleton.hh"
#include "iofwdutil/ThreadPool.hh"

#include "zoidfs/zoidfs.h"

#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"
#include "iofwd/extraservice/aarpc/AtomicAppendServerRPC.hh"

#include <boost/thread.hpp>

#include "iofwdevent/TimerResource.hh"

namespace iofwd
{
    namespace extraservice
    {
        class AtomicAppendManager : public iofwdutil::Singleton<
                                    AtomicAppendManager >
        {
            protected:
                class AtomicAppendBatchData;
                friend class AtomicAppendBatchData;

            public:
                AtomicAppendManager() :
                    aarpc_tp_(iofwdutil::ThreadPool::instance()),
                    rpc_(iofwd::service::ServiceManager::instance().loadService<
                            iofwd::extraservice::AtomicAppendClientRPC>("aarpcclient")),
                    timer_(iofwdevent::TimerResource::instance()),
                    batch_mode_(AtomicAppendServerRPC::batch_mode_),
                    batch_limit_(AtomicAppendServerRPC::batch_limit_),
                    batch_period_(AtomicAppendServerRPC::batch_period_)
                {
                    /* enable the timer resource */
                    timer_.start();
                }

                ~AtomicAppendManager()
                {
                    timer_.stop();
                }

            protected:

                enum AARPCWorkUnitType
                {
                    AA_CREATE_OFFSET_WU = 0,
                    AA_DELETE_OFFSET_WU,
                    AA_GETNEXT_OFFSET_WU,
                    AA_BATCH_GETNEXT_OFFSET_WU,
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

                        AtomicAppendWorkUnit(int type,
                             iofwdutil::ThreadPool & tp,
                             boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc) :
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
       
                /* Get next offset work unit */
                class AtomicAppendBatchGetNextOffsetWorkUnit :
                    public AtomicAppendWorkUnit
                {
                    public:
                        AtomicAppendBatchGetNextOffsetWorkUnit(
                                iofwdutil::ThreadPool & tp,
                                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> rpc,
                                zoidfs::zoidfs_handle_t * h,
                                zoidfs::zoidfs_file_size_t i) :
                            AtomicAppendWorkUnit(AA_BATCH_GETNEXT_OFFSET_WU, tp, rpc),
                            handle(h),
                            incsize(i),
                            offset(0),
                            retcode(0)
                        {
                        }

                        void
                            loadwunits(std::vector<AtomicAppendGetNextOffsetWorkUnit
                                    *> & wunits)
                        {
                                std::vector<AtomicAppendGetNextOffsetWorkUnit
                                    *>::iterator it;

                                /* copy arg into local vec */
                                for(it = wunits.begin() ; it < wunits.end() ;
                                        it++)
                                {
                                    wunits_.push_back(*it);
                                }
                        }

                        void issueBatchCBs(boost::exception_ptr & e)
                        {
                            std::vector<AtomicAppendGetNextOffsetWorkUnit*>::iterator it;
                            zoidfs::zoidfs_file_size_t cur_offset = offset;

                            for(it = wunits_.begin() ; it < wunits_.end() ; it++)
                            {
                                /* update the batch request offset */
                                *((*it)->offset) = cur_offset;
                                cur_offset += (*it)->incsize;

                                /* issue the cb */
                                (*it)->cb_(iofwdevent::CBException(e));

                                /* delete the request */
                                delete (*it);
                            }

                            /* dump the list of batch work units */
                            wunits_.clear();
                        }

                        zoidfs::zoidfs_handle_t * handle;
                        zoidfs::zoidfs_file_size_t incsize;
                        zoidfs::zoidfs_file_size_t offset;
                        uint64_t retcode;
                        std::vector<AtomicAppendGetNextOffsetWorkUnit * > wunits_;
                };
       
                /* handle work unit: submit work unit to thread pool or spawn a thread
                * */ 
                void submitWorkUnit(const boost::function<void (void)> & wu_,
                        iofwdutil::ThreadPool::TPPrio prio, bool use_thread_pool=true)
                {
                    /* if use thread pool option, submit wu onto thread pool */
                    if(use_thread_pool)
                    {
                        aarpc_tp_.submitWorkUnit(wu_, prio,
                                iofwdutil::ThreadPool::RPC);
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

            protected:

                struct HandleCompare
                {
                    bool operator() (const zoidfs::zoidfs_handle_t & a,
                        const zoidfs::zoidfs_handle_t &b)
                    {
                        return memcmp(&a, &b, sizeof(zoidfs::zoidfs_handle_t));
                    }
                };

                boost::mutex batch_handle_mutex_;
                std::map< zoidfs::zoidfs_handle_t, AtomicAppendBatchData *,
                    HandleCompare > batch_handles_;
                iofwdevent::TimerResource & timer_;
                bool batch_mode_;
                int batch_limit_;
                int batch_period_;

            protected:
                class AtomicAppendBatchData
                {
                    protected:
                    public:
                        AtomicAppendBatchData(zoidfs::zoidfs_handle_t * h,
                                int bl,
                                int bp,
                                iofwdutil::ThreadPool & tp,
                                boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC> & rpc,
                                AtomicAppendManager * aa_manager) :
                            handle_(*h),
                            batch_limit_(bl),
                            batch_size_(0),
                            batch_period_(bp),
                            batch_chunk_(0),
                            timer_running_(false),
                            aarpc_tp_(tp),
                            rpc_(rpc),
                            aa_manager_(aa_manager)
                        {
                            /* setup the timer cb for this handle */
                            timer_cb_ =
                                boost::function<void(iofwdevent::CBException)>
                                (boost::bind(&AtomicAppendBatchData::batchTimerCB, this,
                                    _1));
                        }

                        /* callback assumes that timer is running */
                        void batchTimerCB(iofwdevent::CBException e)
                        {
                            boost::mutex::scoped_lock l(batch_mutex_);

                            /* check the exception */
                            e.check();

                            /* create a batch request */
                            AtomicAppendBatchGetNextOffsetWorkUnit * batch_wu = new
                                    AtomicAppendBatchGetNextOffsetWorkUnit(
                                    aarpc_tp_, rpc_, &handle_, batch_chunk_);

                            /* add the CBs for the batch of requests to the bulk
                             * request */
                            batch_wu->loadwunits(batch_wu_items_);
                            batch_wu_items_.clear();

                            /* submit the bulk request */
                            aa_manager_->submitWorkUnit(
                                    boost::bind(
                                        &AtomicAppendManager::runAARPCWorkUnit,
                                        batch_wu),
                                    iofwdutil::ThreadPool::HIGH,
                                    iofwdutil::ThreadPool::RPC);

                            /* reset the batch params */
                            batch_chunk_ = 0;
                            batch_size_ = 0;

                            /* turn the timer off */
                            timer_running_ = false;
                        }

                        /* batch lock */
                        boost::mutex batch_mutex_;

                        /* file target */
                        zoidfs::zoidfs_handle_t handle_;

                        /* batch items */
                        std::vector< AtomicAppendGetNextOffsetWorkUnit * >
                            batch_wu_items_;

                        /* batch params */
                        int batch_limit_;
                        int batch_size_;
                        int batch_period_; /* in ms */
                        int batch_chunk_; /* in bytes */
                       
                        /* timer params */ 
                        iofwdevent::CBType timer_cb_;
                        bool timer_running_;
                        iofwdevent::TimerResource::Handle timer_handle_;

                        iofwdutil::ThreadPool & aarpc_tp_;
                        boost::shared_ptr<iofwd::extraservice::AtomicAppendClientRPC>
                            & rpc_;

                        AtomicAppendManager * aa_manager_;
                };
        };
    }
}

#endif
