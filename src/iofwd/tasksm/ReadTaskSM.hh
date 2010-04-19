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
#include "iofwd/BMIMemoryManager.hh"

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
        ReadTaskSM(sm::SMManager & smm, RequestScheduler * sched, Request * r);
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
                for (size_t i = 0; i < p.mem_count; i++)
                    total_bytes_ += p.mem_sizes[i];

                /* compute the total number of concurrent pipeline ops */
                total_pipeline_ops_ = (int)ceil(1.0 * total_bytes_ / iofwd::BMIMemoryManager::instance().pipeline_size());

                computePipelineFileSegments();

                /* setup the rbuffer variable */
                rbuffer_ = new SMRetrievedBuffer*[total_pipeline_ops_];
                for(int i = 0 ; i < total_pipeline_ops_ ; i++)
                {
                    rbuffer_[i] = new SMRetrievedBuffer(request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND, iofwd::BMIMemoryManager::instance().pipeline_size());
                    rbuffer_[i]->reinit();
                }

                /* transition to the allocate buffer state */
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

            /* dealloc the buffer */
            iofwd::BMIMemoryManager::instance().dealloc(rbuffer_[cw_post_index_]->buffer);

            /* if we still have pipeline data to fetch go back to the allocate buffer state */
            if(cur_sent_bytes_ < total_bytes_)
            {
                /* reset the rbuffer variable */
                rbuffer_[cw_post_index_]->reinit();
                setNextMethod(&ReadTaskSM::postAllocateBMIBuffer);
            }
            else
            {
                setNextMethod(&ReadTaskSM::postReply);
            }
        }

        void postPipelineEnqueueRead(int UNUSED(status))
        {
            p_siz_ = std::min((size_t)iofwd::BMIMemoryManager::instance().pipeline_size(), total_bytes_ - cur_sent_bytes_);
            /* update the rbuffer with the new data entires */
            rbuffer_[cw_post_index_]->siz = p_siz_;
            rbuffer_[cw_post_index_]->off = cur_sent_bytes_;

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
        void readBarrier(int status);
        void computePipelineFileSegments();

        /* @TODO currently set the concurrent pipeline op count to 128... this should be dynamic or tunable */
        enum {READ_SLOT = 0, READ_PIPEOP_START, NUM_READ_SLOTS = 129};
        ReadRequest::ReqParam p;
        RequestScheduler * sched_;
        ReadRequest & request_;
        sm::SimpleSlots<NUM_READ_SLOTS, iofwd::tasksm::ReadTaskSM> slots_;

        /* pipeline variables */
        boost::mutex slot_mutex_;
        size_t total_bytes_;
        size_t cur_sent_bytes_;
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
