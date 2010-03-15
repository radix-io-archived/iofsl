#ifndef __IOFWD_TASKSM_READTASKSM_HH__
#define __IOFWD_TASKSM_READTASKSM_HH__

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/tasksm/SMRetrievedBuffer.hh"
#include "iofwd/ReadRequest.hh"

#include <cstdio>
#include <deque>

namespace iofwd
{
    namespace tasksm
    {
class ReadTaskSM : public sm::SimpleSM< ReadTaskSM >, public iofwdutil::InjectPool< ReadTaskSM >
{
    public:
        ReadTaskSM(sm::SMManager & smm, RequestScheduler * sched, BMIBufferPool * bpool, Request * r);
        ~ReadTaskSM();

        void init(int UNUSED(status))
        {
            setNextMethod(&ReadTaskSM::decodeInputParams);
        }

        void decodeInputParams(int UNUSED(status))
        {
            decodeInput();
            if(p.pipeline_size == 0)
            {
                setNextMethod(&ReadTaskSM::postEnqueueRead);
            }
            else
            {
                /* compute the total size of the pipeline transfers */
                for (uint32_t i = 0; i < p.mem_count; i++)
                    total_bytes_ += p.mem_sizes[i];

                /* reset the rbuffer variable */
                rbuffer_.reinit();
                setNextMethod(&ReadTaskSM::postAllocateBMIBuffer);
            }
        }

        /* normal mode (non-pipeline) state transitions */
        void postSendInputBuffers(int UNUSED(status))
        {
            sendBuffers();
        }

        void waitSendInputBuffers(int UNUSED(status))
        {
            setNextMethod(&ReadTaskSM::postReply);
        }

        void postEnqueueRead(int UNUSED(status))
        {
            readNormal();
        }

        void waitEnqueueRead(int UNUSED(status))
        {
            setNextMethod(&ReadTaskSM::postSendInputBuffers);
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
         *  2) read buffer from disk
         *  3) send pipeline data to client
         *  4) go back to 1) unless all data was sent to the client
         */

        void postAllocateBMIBuffer(int UNUSED(status))
        {
            getBMIBuffer();
        }

        void waitAllocateBMIBuffer(int UNUSED(status))
        {
            setNextMethod(&ReadTaskSM::postPipelineEnqueueRead);
        }

        void postSendPipelineBuffer(int UNUSED(status))
        {
            sendPipelineBuffer();
        }

        void waitSendPipelineBuffer(int UNUSED(status))
        {
            /* update the amount of outstanding data */
            cur_sent_bytes_ += p_siz_;

            /* if we still have pipeline data to fetch go back to the allocate buffer state */
            if(cur_sent_bytes_ < total_bytes_)
            {
                /* reset the rbuffer variable */
                rbuffer_.reinit();
                setNextMethod(&ReadTaskSM::postAllocateBMIBuffer);
            }
            else
            {
                setNextMethod(&ReadTaskSM::postReply);
            }
        }

        void postPipelineEnqueueRead(int UNUSED(status))
        {
            p_siz_ = std::min(bpool_->pipeline_size(), total_bytes_ - cur_sent_bytes_);
            /* update the rbuffer with the new data entires */
            rbuffer_.siz = p_siz_;
            rbuffer_.off = cur_sent_bytes_;

            execPipelineIO();
        }

        void waitPipelineEnqueueRead(int UNUSED(status))
        {
            setNextMethod(&ReadTaskSM::postSendPipelineBuffer);
        }

    protected:

        /* normal mode operations */
        void decodeInput();
        void sendBuffers();
        void readNormal();
        void reply();

        /* pipeline mode operations */
        void getBMIBuffer();
        void sendPipelineBuffer();
        void execPipelineIO();

        enum {READ_SLOT = 0, NUM_READ_SLOTS};
        ReadRequest::ReqParam p;
        BMIBufferPool * bpool_;
        RequestScheduler * sched_;
        ReadRequest & request_;
        sm::SimpleSlots<NUM_READ_SLOTS, iofwd::tasksm::ReadTaskSM> slots_;

        /* pipeline variables */
        uint64_t total_bytes_;
        uint64_t cur_sent_bytes_;
        size_t p_siz_;

        SMRetrievedBuffer rbuffer_;
};
    }
}
#endif
