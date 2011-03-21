#include "iofwd/extraservice/aarpc/AtomicAppendManager.hh"

namespace iofwd
{
    namespace extraservice
    {
        /* static aarpc client */
        iofwd::extraservice::AtomicAppendClientRPC AtomicAppendManager::rpc_;

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
                    rpc_.createOffset(*(wu->handle), *(wu->offset), wu->retcode);
                        
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
                    rpc_.deleteOffset(*(wu->handle), wu->retcode);
                        
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
                    rpc_.getNextOffset(*(wu->handle), wu->incsize, *(wu->offset),
                            wu->retcode);
                        
                    /* invoke the callback */
                    wu->cb_(iofwdevent::CBException(e));

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
                AtomicAppendCreateOffsetWorkUnit(cb, aarpc_tp_, handle,
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
                AtomicAppendDeleteOffsetWorkUnit(cb, aarpc_tp_, handle,
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
                AtomicAppendGetNextOffsetWorkUnit(cb, aarpc_tp_, handle,
                        incsize, offset);

            submitWorkUnit(boost::bind(&AtomicAppendManager::runAARPCWorkUnit, wu),
                                    iofwdutil::ThreadPool::HIGH);
        }
    }
}

