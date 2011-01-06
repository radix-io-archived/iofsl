#ifndef IOFWD_TASKSM_WRITETASKSM_HH
#define IOFWD_TASKSM_WRITETASKSM_HH

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/WriteRequest.hh"

#include "zoidfs/zoidfs.h"

#include <cstdio>
#include <deque>
#include <math.h>

/* mode options */
#define WRITESM_SERIAL_IO_PIPELINE 0
#define WRITESM_PARA_IO_PIPELINE 1

using namespace zoidfs;

namespace iofwd
{
    namespace tasksm
    {

class WriteTaskSM : public sm::SimpleSM< WriteTaskSM >, public
                    iofwdutil::InjectPool< WriteTaskSM >
{
    public:
        WriteTaskSM(sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * r);
        ~WriteTaskSM();

        void init(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&WriteTaskSM::decodeInputParams);
        }

        void decodeInputParams(iofwdevent::CBException e)
        {
           e.check ();
            decodeInput();

            /* compute the total size of the pipeline transfers */
            for (size_t i = 0; i < p.mem_count; i++)
                total_bytes_ += p.mem_sizes[i];

            if(p.pipeline_size == 0)
            {
                /* setup the rbuffer variable */
                rbuffer_ = new RetrievedBuffer*[1];
                rbuffer_[0] = new RetrievedBuffer(total_bytes_);
                total_buffers_ = 1;
                setNextMethod(&WriteTaskSM::postAllocateSingleBuffer);
            }
            else
            {
                /* compute the total number of concurrent pipeline ops */
                pipeline_size_ = std::min(p.max_buffer_size, p.pipeline_size);
                total_pipeline_ops_ = (int)ceil(1.0 * total_bytes_ / pipeline_size_);
                total_buffers_ = total_pipeline_ops_;

                computePipelineFileSegments();

                /* setup the rbuffer variable */
                bool lastBufferIsPartial = (total_bytes_ % pipeline_size_ == 0 ? false : true);
                rbuffer_ = new RetrievedBuffer*[total_pipeline_ops_];
                for(int i = 0 ; i < total_pipeline_ops_ ; i++)
                {
                    if(lastBufferIsPartial && i + 1 == total_pipeline_ops_)
                    {
                        rbuffer_[i] = new RetrievedBuffer(total_bytes_ % pipeline_size_);
                    }
                    else
                    {
                        rbuffer_[i] = new RetrievedBuffer(pipeline_size_);
                    }
                }
    
                /* transition to the allocate buffer state */
                setNextMethod(&WriteTaskSM::postAllocateBuffer);
            }
        }
    
        void postAllocateSingleBuffer(iofwdevent::CBException e)
        {
           e.check ();
            getSingleBuffer();
        }

        void waitAllocateSingleBuffer(iofwdevent::CBException e)
        {
           e.check ();
            // init the task params
            request_.initRequestParams(p, rbuffer_[0]->buffer_->getMemory());

            setNextMethod(&WriteTaskSM::postRecvInputBuffers);
        }

        /* normal mode (non-pipeline) state transitions */
        void postRecvInputBuffers(iofwdevent::CBException e)
        {
           e.check ();
            recvBuffers();
        }

        void waitRecvInputBuffers(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&WriteTaskSM::postEnqueueWrite);
        }

        void postEnqueueWrite(iofwdevent::CBException e)
        {
           e.check ();
            writeNormal();
        }

        void waitEnqueueWrite(iofwdevent::CBException e)
        {
           e.check ();
            /* free the buffer */
            request_.releaseBuffer(rbuffer_[0]);

            setNextMethod(&WriteTaskSM::postReply);
        }

        void postReply(iofwdevent::CBException e)
        {
           e.check ();
            reply();
        }

        void waitReply(iofwdevent::CBException e)
        {
           e.check ();
            // done...
        }

        /* pipeline mode state transistions */

        /*
         * below is a very simple pipeline state machine...
         *  1) allocate a buffer
         *  2) recv pipeline data into buffer
         *  3) write buffer to disk
         *  4) go back to 1) unless all data was recv'ed from the client
         */

        void postAllocateBuffer(iofwdevent::CBException e)
        {
           e.check ();
            getBuffer();
        }

        void waitAllocateBuffer(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&WriteTaskSM::postRecvPipelineBuffer);
        }

        void postRecvPipelineBuffer(iofwdevent::CBException e)
        {
           e.check ();
            recvPipelineBuffer();
        }

        void waitRecvPipelineBuffer(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&WriteTaskSM::postPipelineEnqueueWrite);
        }

        void postPipelineEnqueueWrite(iofwdevent::CBException e)
        {
           e.check ();
            /* update the rbuffer with the new data entires */
            rbuffer_[cw_post_index_]->siz = p_siz_;
            rbuffer_[cw_post_index_]->off = cur_recv_bytes_;

            execPipelineIO();
        }

        void waitPipelineEnqueueWrite(iofwdevent::CBException e)
        {
           e.check ();
            if(mode_ == WRITESM_SERIAL_IO_PIPELINE)
            {
                /* update the return code */
                if(*(rbuffer_[cw_post_index_]->ret) != zoidfs::ZFS_OK)
                {
                    ret_ = *(rbuffer_[cw_post_index_]->ret);
                }

                /* update the amount of outstanding data */
                cur_recv_bytes_ += p_siz_;

                /* dealloc the buffer */
                request_.releaseBuffer(rbuffer_[cw_post_index_]);
                cw_post_index_++;

                /* if we still have pipeline data to fetch go back to the allocate buffer state */
                if(cur_recv_bytes_ < total_bytes_)
                {
                    /* reset the rbuffer variable */
                    setNextMethod(&WriteTaskSM::postAllocateBuffer);
                }
                /* else, we are done... reply to the client */
                else
                {
                    setNextMethod(&WriteTaskSM::postReply);
                }
            }
            else
            {
                boost::mutex::scoped_lock l(slot_mutex_);

                /* if we still have pipeline data to fetch go back to the allocate buffer state */
                if(cur_recv_bytes_ < total_bytes_)
                {
                    /* reset the rbuffer variable */
                    setNextMethod(&WriteTaskSM::postAllocateBuffer);
                }
            }
        }

    protected:

        /* normal mode operations */
        void decodeInput();
        void recvBuffers();
        void writeNormal();
        void reply();

        /* pipeline mode operations */
        void getBuffer();
        void getSingleBuffer();
        void recvPipelineBuffer();
        void execPipelineIO();
        void waitWriteBarrier(iofwdevent::CBException e);
        void writeDoneCB(iofwdevent::CBException status, int my_slot);

        void computePipelineFileSegments();

        enum {WRITE_SLOT = 0, NUM_WRITE_SLOTS = 1};
        WriteRequest::ReqParam p;
        zoidfs::util::ZoidFSAsync * api_;
        WriteRequest & request_;
        sm::SimpleSlots<NUM_WRITE_SLOTS, iofwd::tasksm::WriteTaskSM> slots_;

        /* pipeline variables */
        boost::mutex slot_mutex_;
        size_t total_bytes_;
        size_t cur_recv_bytes_;
        size_t p_siz_;
        int total_pipeline_ops_;
        int total_buffers_;
        int io_ops_done_;
        int cw_post_index_;

        RetrievedBuffer ** rbuffer_;

        unsigned int mode_;

        std::vector<zoidfs_file_size_t> p_file_sizes;
        std::vector<zoidfs_file_ofs_t> p_file_starts;
        std::vector<size_t> p_mem_offsets;
        std::vector<int> p_segments;
        std::vector<int> p_segments_start;

        int ret_;

        iofwdevent::CBType s_;

        size_t pipeline_size_;
};
    }
}
#endif
