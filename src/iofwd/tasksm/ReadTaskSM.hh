#ifndef IOFWD_TASKSM_READTASKSM_HH
#define IOFWD_TASKSM_READTASKSM_HH

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/ReadRequest.hh"

#include "zoidfs/zoidfs.h"

#include <cstdio>
#include <deque>

#include <math.h>

/* mode options */
#define READSM_SERIAL_IO_PIPELINE 0
//#define READSM_PARA_IO_PIPELINE 1 /* @TODO fix this mode */

using namespace zoidfs;

namespace iofwd
{
    namespace tasksm
    {
class ReadTaskSM : public sm::SimpleSM< ReadTaskSM >, public iofwdutil::InjectPool< ReadTaskSM >
{
    public:
        ReadTaskSM(sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * r);
        ~ReadTaskSM();

        void init(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&ReadTaskSM::decodeInputParams);
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
                setNextMethod(&ReadTaskSM::postAllocateSingleBuffer);
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
                setNextMethod(&ReadTaskSM::postAllocateBuffer);
            }
        }

        /* normal mode (non-pipeline) state transitions */
        void postAllocateSingleBuffer(iofwdevent::CBException e)
        {
           e.check ();
            getSingleBuffer();
        }

        void waitAllocateSingleBuffer(iofwdevent::CBException e)
        {
           e.check ();
            request_.initRequestParams(p, rbuffer_[0]->buffer_->getMemory());

            setNextMethod(&ReadTaskSM::postEnqueueRead);
        }

        void postSendInputBuffers(iofwdevent::CBException e)
        {
           e.check ();
            sendBuffers();
        }

        void waitSendInputBuffers(iofwdevent::CBException e)
        {
           e.check ();
            /* free the buffer */
            request_.releaseBuffer(rbuffer_[0]);

            setNextMethod(&ReadTaskSM::postReply);
        }

        void postEnqueueRead(iofwdevent::CBException e)
        {
           e.check ();
            readNormal();
        }

        void waitEnqueueRead(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&ReadTaskSM::postSendInputBuffers);
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
         *  2) read buffer from disk
         *  3) send pipeline data to client
         *  4) go back to 1) unless all data was sent to the client
         */

        void postAllocateBuffer(iofwdevent::CBException e)
        {
           e.check ();
            getBuffer();
        }

        void waitAllocateBuffer(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&ReadTaskSM::postPipelineEnqueueRead);
        }

        void postSendPipelineBuffer(iofwdevent::CBException e)
        {
           e.check ();
            sendPipelineBuffer();
        }

        void waitSendPipelineBuffer(iofwdevent::CBException e)
        {
           e.check ();
            /* update the return code */
            if(*(rbuffer_[cw_post_index_]->ret) != zoidfs::ZFS_OK)
            {
                ret_ = *(rbuffer_[cw_post_index_]->ret);
            }

            /* update the amount of outstanding data */
            cur_sent_bytes_ += p_siz_;

            /* dealloc the buffer */
            request_.releaseBuffer(rbuffer_[cw_post_index_]);

            /* update the index counter */
            cw_post_index_++;

            /* if we still have pipeline data to fetch go back to the allocate buffer state */
            if(cur_sent_bytes_ < total_bytes_)
            {
                /* reset the rbuffer variable */
                setNextMethod(&ReadTaskSM::postAllocateBuffer);
            }
            else
            {
                setNextMethod(&ReadTaskSM::postReply);
            }
        }

        void postPipelineEnqueueRead(iofwdevent::CBException e)
        {
           e.check ();
            p_siz_ = std::min(pipeline_size_, total_bytes_ - cur_sent_bytes_);
            /* update the rbuffer with the new data entires */
            rbuffer_[cw_post_index_]->siz = p_siz_;
            rbuffer_[cw_post_index_]->off = cur_sent_bytes_;

            execPipelineIO();
        }

        void waitPipelineEnqueueRead(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&ReadTaskSM::postSendPipelineBuffer);
        }

    protected:

        /* normal mode operations */
        void decodeInput();
        void sendBuffers();
        void readNormal();
        void reply();

        /* pipeline mode operations */
        void getBuffer();
        void getSingleBuffer();
        void sendPipelineBuffer();
        void execPipelineIO();
        void readBarrier(iofwdevent::CBException e);
        void computePipelineFileSegments();

        /* @TODO currently set the concurrent pipeline op count to 128... this should be dynamic or tunable */
        enum {READ_SLOT = 0, READ_PIPEOP_START, NUM_READ_SLOTS = 129};
        ReadRequest::ReqParam p;
        zoidfs::util::ZoidFSAsync * api_;
        ReadRequest & request_;
        sm::SimpleSlots<NUM_READ_SLOTS, iofwd::tasksm::ReadTaskSM> slots_;

        /* pipeline variables */
        boost::mutex slot_mutex_;
        size_t total_bytes_;
        size_t cur_sent_bytes_;
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

        size_t pipeline_size_;
};
    }
}
#endif
