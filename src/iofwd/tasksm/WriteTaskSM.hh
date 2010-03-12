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

#include <cstdio>
#include <deque>

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
                for (uint32_t i = 0; i < p.mem_count; i++)
                    total_bytes_ += p.mem_sizes[i];

                /* reset the rbuffer variable */
                rbuffer_.reinit();
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
            rbuffer_.siz = p_siz_;
            rbuffer_.off = cur_recv_bytes_;

            execPipelineIO();
        }

        void waitPipelineEnqueueWrite(int UNUSED(status))
        {
            /* update the amount of outstanding data */
            cur_recv_bytes_ += p_siz_;

            /* if we still have pipeline data to fetch go back to the allocate buffer state */
            if(cur_recv_bytes_ < total_bytes_)
            {
                /* reset the rbuffer variable */
                rbuffer_.reinit();
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

        enum {WRITE_SLOT = 0, NUM_WRITE_SLOTS};
        WriteRequest::ReqParam p;
        RequestScheduler * sched_;
        BMIBufferPool * bpool_;
        WriteRequest & request_;
        sm::SimpleSlots<NUM_WRITE_SLOTS, iofwd::tasksm::WriteTaskSM> slots_;

        /* pipeline variables */
        uint64_t total_bytes_;
        uint64_t cur_recv_bytes_;
        size_t p_siz_;

        SMRetrievedBuffer rbuffer_;
};
    }
}
#endif
