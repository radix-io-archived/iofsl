#ifndef __IOFWD_TASKSM_WRITETASKSM_HH__
#define __IOFWD_TASKSM_WRITETASKSM_HH__

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/tasksm/SMRetrievedBuffer.hh"
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

class WriteTaskSM : public sm::SimpleSM< WriteTaskSM >, public iofwdutil::InjectPool< WriteTaskSM >
{
    public:
        WriteTaskSM(sm::SMManager & smm, RequestScheduler * sched, BMIBufferPool * bpool, Request * r);
        ~WriteTaskSM();

        void init(int UNUSED(status))
        {
            setNextMethod(&WriteTaskSM::decodeInputParams);
        }

        void decodeInputParams(int UNUSED(status))
        {
            decodeInput();
            if(p.pipeline_size == 0)
            {
                setNextMethod(&WriteTaskSM::postRecvInputBuffers);
            }
            else
            {
                /* compute the total size of the pipeline transfers */
                for (size_t i = 0; i < p.mem_count; i++)
                    total_bytes_ += p.mem_sizes[i];

                /* compute the total number of concurrent pipeline ops */
                total_pipeline_ops_ = (int)ceil(1.0 * total_bytes_ / bpool_->pipeline_size());

                computePipelineFileSegments();

                /* setup the rbuffer variable */
                rbuffer_ = new SMRetrievedBuffer*[total_pipeline_ops_];
                for(int i = 0 ; i < total_pipeline_ops_ ; i++)
                {
                    rbuffer_[i] = new SMRetrievedBuffer();
                    rbuffer_[i]->reinit();
                }

                /* transition to the allocate buffer state */
                setNextMethod(&WriteTaskSM::postAllocateBMIBuffer);
            }
        }

        /* normal mode (non-pipeline) state transitions */
        void postRecvInputBuffers(int UNUSED(status))
        {
            recvBuffers();
        }

        void waitRecvInputBuffers(int UNUSED(status))
        {
            setNextMethod(&WriteTaskSM::postEnqueueWrite);
        }

        void postEnqueueWrite(int UNUSED(status))
        {
            writeNormal();
        }

        void waitEnqueueWrite(int UNUSED(status))
        {
            setNextMethod(&WriteTaskSM::postReply);
        }

        void postReply(int UNUSED(status))
        {
            reply();
        }

        void waitReply(int UNUSED(status))
        {
            // done...
        }

        /* pipeline mode state transistions */

        /*
         * below is a very simple pipeline state machine...
         *  1) allocate a BMI buffer
         *  2) recv pipeline data into buffer
         *  3) write buffer to disk
         *  4) go back to 1) unless all data was recv'ed from the client
         */

        void postAllocateBMIBuffer(int UNUSED(status))
        {
            getBMIBuffer();
        }

        void waitAllocateBMIBuffer(int UNUSED(status))
        {
            setNextMethod(&WriteTaskSM::postRecvPipelineBuffer);
        }

        void postRecvPipelineBuffer(int UNUSED(status))
        {
            recvPipelineBuffer();
        }

        void waitRecvPipelineBuffer(int UNUSED(status))
        {
            setNextMethod(&WriteTaskSM::postPipelineEnqueueWrite);
        }

        void postPipelineEnqueueWrite(int UNUSED(status))
        {
            /* update the rbuffer with the new data entires */
            rbuffer_[cw_post_index_]->siz = p_siz_;
            rbuffer_[cw_post_index_]->off = cur_recv_bytes_;

            execPipelineIO();
        }

        void waitPipelineEnqueueWrite(int UNUSED(status))
        {
            /* update the amount of outstanding data */
            cur_recv_bytes_ += p_siz_;
            cw_post_index_++;

            /* if we still have pipeline data to fetch go back to the allocate buffer state */
            if(cur_recv_bytes_ < total_bytes_)
            {
                /* reset the rbuffer variable */
                rbuffer_[cw_post_index_]->reinit();
                setNextMethod(&WriteTaskSM::postAllocateBMIBuffer);
            }
            /* else, we are done... reply to the client */
            else
            {
                setNextMethod(&WriteTaskSM::postReply);
            }
        }

    protected:

        /* normal mode operations */
        void decodeInput();
        void recvBuffers();
        void writeNormal();
        void reply();

        /* pipeline mode operations */
        void getBMIBuffer();
        void recvPipelineBuffer();
        void execPipelineIO();
        void writeBarrier(int status);

        void computePipelineFileSegments();

        /* set the concurrent pipeline op count to 128... this should be dynamic or tunable */
        enum {WRITE_SLOT = 0, WRITE_PIPEOP_START, NUM_WRITE_SLOTS = 129};
        WriteRequest::ReqParam p;
        RequestScheduler * sched_;
        BMIBufferPool * bpool_;
        WriteRequest & request_;
        sm::SimpleSlots<NUM_WRITE_SLOTS, iofwd::tasksm::WriteTaskSM> slots_;

        /* pipeline variables */
        boost::mutex slot_mutex_;
        size_t total_bytes_;
        size_t cur_recv_bytes_;
        size_t p_siz_;
        int total_pipeline_ops_;
        int io_ops_done_;
        int cw_post_index_;

        SMRetrievedBuffer ** rbuffer_;

        unsigned int mode_;

        std::vector<zoidfs_file_size_t> p_file_sizes;
        std::vector<zoidfs_file_ofs_t> p_file_starts;
        std::vector<size_t> p_mem_offsets;
        std::vector<int> p_segments;
        std::vector<int> p_segments_start;
};
    }
}
#endif
