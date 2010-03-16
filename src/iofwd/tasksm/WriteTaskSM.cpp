#include "iofwd/tasksm/WriteTaskSM.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd/RequestScheduler.hh"
#include "iofwd/BMIBufferPool.hh"

namespace iofwd
{
    namespace tasksm
    {

    WriteTaskSM::WriteTaskSM(sm::SMManager & smm, RequestScheduler * sched, BMIBufferPool * bpool, Request * p)
        : sm::SimpleSM<WriteTaskSM>(smm), sched_(sched), bpool_(bpool), request_((static_cast<WriteRequest&>(*p))), slots_(*this),
          total_bytes_(0), cur_recv_bytes_(0), p_siz_(0), total_pipeline_ops_(0), io_ops_done_(0), cw_post_index_(0), rbuffer_(NULL), mode_(WRITESM_PARA_IO_PIPELINE)
    {
    }

    WriteTaskSM::~WriteTaskSM()
    {
        delete &request_;

        /* cleanup rbuffer_ wrappers */
        for(uint64_t i = 0 ; i < total_pipeline_ops_ ; i++)
        {
            delete rbuffer_[i];
        }
        delete [] rbuffer_;
    }

    void WriteTaskSM::decodeInput()
    {
        p = request_.decodeParam();
    }

    /* recv the input data to write to the disk */
    void WriteTaskSM::recvBuffers()
    {
        /* issue the recv buffer */
        request_.recvBuffers(slots_[WRITE_SLOT]);

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitRecvInputBuffers);
    }

    /* execute the write I/O path */
    void WriteTaskSM::writeNormal()
    {
        // p.mem_sizes is uint64_t array, but ZoidFSAPI::write() takes size_t array
        // for its arguments. Therefore, in (sizeof(size_t) != sizeof(uint64_t))
        // environment (32bit), p.mem_sizes is not valid for size_t array.
        // We allocate temporary buffer to fix this problem.
        size_t * tmp_mem_sizes = (size_t*)p.mem_sizes;
        bool need_size_t_workaround = (sizeof(size_t) != sizeof(uint64_t));
        if (need_size_t_workaround)
        {
            tmp_mem_sizes = new size_t[p.mem_count];
            for (uint32_t i = 0; i < p.mem_count; i++)
                tmp_mem_sizes[i] = p.mem_sizes[i];
        }

#if SIZEOF_SIZE_T == SIZEOF_INT64_T
        sched_->enqueueWriteCB(slots_[WRITE_SLOT], p.handle, (size_t)p.mem_count, (const void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
#else
        sched_->enqueueWriteCB(slots_[WRITE_SLOT], p.handle, (size_t)p.mem_count, (const void**)p.mem_starts, p.bmi_mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
#endif

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitEnqueueWrite);
    }

    /* issue the reply */
    void WriteTaskSM::reply()
    {
        /* set the return code */
        request_.setReturnCode(zoidfs::ZFS_OK);

        /* issue the reply */
        request_.reply(slots_[WRITE_SLOT]);

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitReply);
    }

    void WriteTaskSM::execPipelineIO()
    {
        const char * p_buf = (char *)rbuffer_[cw_post_index_]->buffer->get_buf()->get();
        const uint64_t p_offset = rbuffer_[cw_post_index_]->off;
        const uint64_t p_size = rbuffer_[cw_post_index_]->siz;
        const uint64_t * file_starts = p.file_starts;
        const uint64_t * file_sizes = p.file_sizes;
        int * ret = new int(0);

        uint32_t st_file = 0, en_file = 0;
        uint64_t st_fileofs = 0, en_fileofs = 0;
        {
            bool st_ok = false;
            bool en_ok = false;
            uint32_t cur_file = 0;
            uint64_t cur_ofs = p_offset;
            while (!(st_ok && en_ok)) {
                const uint64_t st = p_offset;
                const uint64_t en = p_offset + p_size;
                assert(cur_file < p.file_count);
                if (cur_ofs <= st && st < cur_ofs + file_sizes[cur_file]) {
                    st_file = cur_file;
                    st_fileofs = st;
                    st_ok = true;
                }
                if (cur_ofs < en && en <= cur_ofs + file_sizes[cur_file]) {
                    en_file = cur_file;
                    en_fileofs = en;
                    en_ok = true;
                    assert(st_ok);
                    break;
                }
                cur_ofs += file_sizes[cur_file];
                cur_file++;
        }
        assert(st_file <= en_file);
    }

    size_t p_file_count = en_file + 1 - st_file;
    uint64_t * p_file_starts = new uint64_t[p_file_count];
    uint64_t * p_file_sizes = new uint64_t[p_file_count];
    if (st_file == en_file) {
        p_file_starts[0] = file_starts[st_file] + st_fileofs;
        assert(en_fileofs > st_fileofs);
        p_file_sizes[0] = en_fileofs - st_fileofs;
    } else {
        for (uint32_t i = st_file; i <= en_file; i++) {
            if (i == st_file) {
                p_file_starts[i - st_file] = file_starts[i] + st_fileofs;
                p_file_sizes[i - st_file] = file_sizes[i] - st_fileofs;
            } else if (i == en_file) {
                p_file_starts[i - st_file] = file_starts[i];
                p_file_sizes[i - st_file] = en_fileofs;
            } else {
                p_file_starts[i - st_file] = file_starts[i];
                p_file_sizes[i - st_file] = file_sizes[i];
            }
        }
    }

    // issue async I/O
    uint64_t cur = 0;
    const char ** mem_starts = new const char*[p_file_count];
    size_t * mem_sizes = new size_t[p_file_count];
    for (size_t i = 0; i < p_file_count; i++) {
        mem_starts[i] = p_buf + cur;
        mem_sizes[i] = p_file_sizes[i];
        cur += p_file_sizes[i];
    }

    rbuffer_[cw_post_index_]->mem_starts = mem_starts;
    rbuffer_[cw_post_index_]->mem_sizes = mem_sizes;
    rbuffer_[cw_post_index_]->file_starts = p_file_starts;
    rbuffer_[cw_post_index_]->file_sizes = p_file_sizes;
    rbuffer_[cw_post_index_]->ret = ret;

    /* set the callback and wait */
    if((mode_ & WRITESM_SERIAL_IO_PIPELINE) == 0)
    {
        /* enqueue the write */
        sched_->enqueueWriteCB (
            slots_[WRITE_SLOT], p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
            p_file_starts, p_file_sizes, p.op_hint);

        /* issue the serial wait */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitPipelineEnqueueWrite);
    }
    else
    {
        int my_slot = 0;

        /* protected section for the op slot update */
        {
            boost::mutex::scoped_lock l(slot_mutex_);

            /* assign the current slot */
            my_slot = cw_post_index_ + WRITE_PIPEOP_START;

            cw_post_index_++;

            /* update the byte transfer count */
            cur_recv_bytes_ += p_siz_;
        }

        /* enqueue the write */
        sched_->enqueueWriteCB (
            slots_[my_slot], p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
            p_file_starts, p_file_sizes, p.op_hint);


        /* go to the next state depending on the current pipeline stage */
        {
            boost::mutex::scoped_lock l(slot_mutex_);

            /* if there is still outstanding pipeline data, go back to the buffer allocate stage */
            if(cur_recv_bytes_ < total_bytes_)
            {
                /* issue the barrier wait */
                rbuffer_[cw_post_index_]->reinit();
                slots_.wait(my_slot, &WriteTaskSM::writeBarrier);
                setNextMethod(&WriteTaskSM::postAllocateBMIBuffer);
            }
            /* else, wait for the barrier */
            else
            {
                slots_.wait(my_slot, &WriteTaskSM::writeBarrier);
            }
        }
    }
}

/* barrier for writes... will not go to post reply state until all writes complete */
void WriteTaskSM::writeBarrier(int UNUSED(status))
{
    boost::mutex::scoped_lock l(slot_mutex_);

    io_ops_done_++;

    /* if all the outstanding ops are done, go to the reply state */
    if(io_ops_done_ == total_pipeline_ops_)
    {
        setNextMethod(&WriteTaskSM::postReply);
    }
}

void WriteTaskSM::getBMIBuffer()
{
    /* request a BMI buffer */
#ifdef USE_IOFWD_TASK_POOL
    bpool_->allocCB(slots_[WRITE_SLOT], request_->getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_RECEIVE, rbuffer_[cw_post_index_]->buffer);
#else
    bpool_->allocCB(slots_[WRITE_SLOT], request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_RECEIVE, rbuffer_[cw_post_index_]->buffer);
#endif

    /* set the callback and wait */
    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitAllocateBMIBuffer);
}

void WriteTaskSM::recvPipelineBuffer()
{
    // The life cycle of buffers is like follows:
    // from alloc -> NetworkRecv -> rx_q -> ZoidI/O -> io_q -> back to alloc

    /* if there is still data to be recieved */
    p_siz_ = std::min(bpool_->pipeline_size(), total_bytes_ - cur_recv_bytes_);
#ifdef USE_IOFWD_TASK_POOL
    request_->recvPipelineBufferCB(slots_[WRITE_SLOT], rbuffer_[cw_post_index_]->buffer->get_buf(), p_siz_);
#else
    request_.recvPipelineBufferCB(slots_[WRITE_SLOT], rbuffer_[cw_post_index_]->buffer->get_buf(), p_siz_);
#endif

    /* set the callback and wait */
    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitRecvPipelineBuffer);
}
    }
}
