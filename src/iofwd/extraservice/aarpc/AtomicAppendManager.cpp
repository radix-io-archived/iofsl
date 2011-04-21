#include "iofwd/extraservice/aarpc/AtomicAppendManager.hh"

#include "iofwd/service/ServiceManager.hh"

namespace iofwd
{
    namespace extraservice
    {
        void AtomicAppendManager::runAARPCWorkUnit(AtomicAppendWorkUnit * aawu)
        {
            boost::exception_ptr e;
            /* based on the wu type */
            switch(aawu->type_)
            {
                /* creates */
                case AA_CREATE_OFFSET_WU:
                {
                    /* cast the wu to the correct type */
                    AtomicAppendCreateOffsetWorkUnit * wu =
                        static_cast<AtomicAppendCreateOffsetWorkUnit *>(aawu);
                        
                    /* invoke the rpc */
                    //wu->rpc_->createOffset(*(wu->handle), *(wu->offset), wu->retcode);
                        
                    /* invoke the callback */
                    wu->cb_(iofwdevent::CBException(e));
                        
                    break;
                }
                /* deletes */
                case AA_DELETE_OFFSET_WU:
                {
                    /* cast the wu to the correct type */
                    AtomicAppendDeleteOffsetWorkUnit * wu =
                        static_cast<AtomicAppendDeleteOffsetWorkUnit *>(aawu);
                        
                    /* invoke the rpc */
                    //wu->rpc_->deleteOffset(*(wu->handle), wu->retcode);
                        
                    /* invoke the callback */
                    wu->cb_(iofwdevent::CBException(e));

                    break;
                }
                /* get next */
                case AA_GETNEXT_OFFSET_WU:
                {
                    /* cast the wu to the correct type */
                    AtomicAppendGetNextOffsetWorkUnit * wu =
                        static_cast<AtomicAppendGetNextOffsetWorkUnit *>(aawu);
                        
                    /* invoke the rpc */
                    wu->rpc_->getNextOffset(*(wu->handle), wu->incsize, *(wu->offset),
                            wu->retcode);
                        
                    /* invoke the callback */
                    wu->cb_(iofwdevent::CBException(e));

                    break;
                }
                /* get next batch mode */
                case AA_BATCH_GETNEXT_OFFSET_WU:
                {
                    /* cast the wu to the correct type */
                    AtomicAppendBatchGetNextOffsetWorkUnit * wu =
                        static_cast<AtomicAppendBatchGetNextOffsetWorkUnit *>(aawu);
                        
                    /* invoke the rpc */
                    wu->rpc_->getNextOffset(*(wu->handle), wu->incsize, (wu->offset),
                            wu->retcode);
                        
                    /* invoke the callback */
                    wu->issueBatchCBs(e);

                    break;
                }
                /* unknown */
                default:
                {
                    aawu->cb_(iofwdevent::CBException(e));
                    break;
                }
            };

#ifndef USE_CRAY_TP
            /* reschedule the thread with more work from the tp */
            boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(aawu->tp_));
#endif
            /* cleanup the wu */
            delete aawu;
        }

        void AtomicAppendManager::issueCreateOffsetRPC(
                const iofwdevent::CBType & cb,
                zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_file_size_t * offset)
        {
            AtomicAppendCreateOffsetWorkUnit * wu = new
                AtomicAppendCreateOffsetWorkUnit(cb, aarpc_tp_, rpc_, handle,
                        offset);

            submitWorkUnit(boost::bind(&AtomicAppendManager::runAARPCWorkUnit, wu),
                                    iofwdutil::ThreadPool::HIGH);
        }

        void AtomicAppendManager::issueDeleteOffsetRPC(
                const iofwdevent::CBType & cb,
                zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_file_size_t * offset)
        {
            AtomicAppendDeleteOffsetWorkUnit * wu = new
                AtomicAppendDeleteOffsetWorkUnit(cb, aarpc_tp_, rpc_, handle,
                        offset);

            submitWorkUnit(boost::bind(&AtomicAppendManager::runAARPCWorkUnit, wu),
                                    iofwdutil::ThreadPool::HIGH);
        }

        void AtomicAppendManager::issueGetNextOffsetRPC(
                const iofwdevent::CBType & cb, 
                zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_file_size_t incsize,
                zoidfs::zoidfs_file_size_t * offset)
        {
            AtomicAppendGetNextOffsetWorkUnit * wu = new
                AtomicAppendGetNextOffsetWorkUnit(cb, aarpc_tp_, rpc_, handle,
                        incsize, offset);

            {
                /* if we are not in the batch mode, submit an rpc requests */
                if(!batch_mode_)
                {
                    submitWorkUnit(boost::bind(&AtomicAppendManager::runAARPCWorkUnit, wu),
                                    iofwdutil::ThreadPool::HIGH);
                }
                /* we're in batching mode */
                else
                {
                    boost::mutex::scoped_lock l(batch_mutex_);
                    /* we have not reached the batch size limit */
                    if(batch_size_ < batch_limit_ - 1)
                    {
                        /* update the batch params */
                        batch_size_++;
                        batch_chunk_ += incsize;
                        batch_wu_items_.push_back(wu);
                    }
                    /* if this request will exceed the threshold */
                    else
                    {
                        /* update the batch params */
                        batch_size_++;
                        batch_chunk_ += incsize;
                        batch_wu_items_.push_back(wu);

                        /* create a batch request */
                        AtomicAppendBatchGetNextOffsetWorkUnit * batch_wu = new
                            AtomicAppendBatchGetNextOffsetWorkUnit(
                                    aarpc_tp_, rpc_, handle, batch_chunk_);

                        /* add the CBs for the batch of requests to the bulk
                         * request */
                        batch_wu->loadwunits(batch_wu_items_);
                        batch_wu_items_.clear();

                        /* submit the bulk request */ 
                        submitWorkUnit(boost::bind(&AtomicAppendManager::runAARPCWorkUnit,
                                    batch_wu), iofwdutil::ThreadPool::HIGH);

                        /* reset the batch params */
                        batch_chunk_ = 0;
                        batch_size_ = 0;
                    }
                }
            }
        }
    }
}

