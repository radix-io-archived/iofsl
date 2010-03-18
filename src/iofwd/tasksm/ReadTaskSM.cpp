#include "iofwd/tasksm/ReadTaskSM.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd/RequestScheduler.hh"
#include "iofwd/BMIBufferPool.hh"

namespace iofwd
{
    namespace tasksm
    {

    ReadTaskSM::ReadTaskSM(sm::SMManager & smm, RequestScheduler * sched, BMIBufferPool * bpool, Request * p)
        : sm::SimpleSM<ReadTaskSM>(smm), bpool_(bpool), sched_(sched), request_((static_cast<ReadRequest&>(*p))), slots_(*this),
          total_bytes_(0), cur_sent_bytes_(0), p_siz_(0), total_pipeline_ops_(0), io_ops_done_(0), cw_post_index_(0), rbuffer_(NULL), mode_(READSM_SERIAL_IO_PIPELINE)
    {
    }

    ReadTaskSM::~ReadTaskSM()
    {
        /* cleanup rbuffer_ wrappers */
        for(uint64_t i = 0 ; i < total_pipeline_ops_ ; i++)
        {
            delete rbuffer_[i];
        }
        delete [] rbuffer_;

        /* delete the request last */
        delete &request_;
    }

    void ReadTaskSM::decodeInput()
    {
        p = request_.decodeParam();
    }

    /* send the input data to write to the disk */
    void ReadTaskSM::sendBuffers()
    {
        /* issue the send buffer */
        request_.sendBuffers(slots_[READ_SLOT]);

        /* set the callback */
        slots_.wait(READ_SLOT, &ReadTaskSM::waitSendInputBuffers);
    }

    /* execute the write I/O path */
    void ReadTaskSM::readNormal()
    {
        sched_->enqueueReadCB(slots_[READ_SLOT], p.handle, (size_t)p.mem_count, (void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);

        /* set the callback */
        slots_.wait(READ_SLOT, &ReadTaskSM::waitEnqueueRead);
    }

    /* issue the reply */
    void ReadTaskSM::reply()
    {
        /* set the return code */
        request_.setReturnCode(zoidfs::ZFS_OK);

        /* issue the reply */
        request_.reply(slots_[READ_SLOT]);

        /* set the callback */
        slots_.wait(READ_SLOT, &ReadTaskSM::waitReply);
    }

    void ReadTaskSM::execPipelineIO()
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
    if((mode_ & READSM_SERIAL_IO_PIPELINE) == 0)
    {
        /* enqueue the write */
        sched_->enqueueReadCB (
            slots_[READ_SLOT], p.handle, p_file_count, (void**)mem_starts, mem_sizes,
            p_file_starts, p_file_sizes, p.op_hint);

        /* issue the serial wait */
        slots_.wait(READ_SLOT, &ReadTaskSM::waitPipelineEnqueueRead);
    }
    else
    {
        int my_slot = 0;

        /* protected section for the op slot update */
        {
            boost::mutex::scoped_lock l(slot_mutex_);

            /* assign the current slot */
            my_slot = cw_post_index_ + READ_PIPEOP_START;

            cw_post_index_++;

            /* update the byte transfer count */
            cur_sent_bytes_ += p_siz_;
        }

        /* enqueue the write */
        sched_->enqueueReadCB (
            slots_[my_slot], p.handle, p_file_count, (void**)mem_starts, mem_sizes,
            p_file_starts, p_file_sizes, p.op_hint);


        /* go to the next state depending on the current pipeline stage */
        {
            boost::mutex::scoped_lock l(slot_mutex_);

            /* if there is still outstanding pipeline data, go back to the buffer allocate stage */
            if(cur_sent_bytes_ < total_bytes_)
            {
                /* issue the barrier wait */
                rbuffer_[cw_post_index_]->reinit();
                slots_.wait(my_slot, &ReadTaskSM::readBarrier);
                setNextMethod(&ReadTaskSM::postAllocateBMIBuffer);
            }
            /* else, wait for the barrier */
            else
            {
                slots_.wait(my_slot, &ReadTaskSM::readBarrier);
            }
        }
    }
}

/* barrier for reads... will not go to post reply state until all writes complete */
void ReadTaskSM::readBarrier(int UNUSED(status))
{
    boost::mutex::scoped_lock l(slot_mutex_);

    io_ops_done_++;

    /* if all the outstanding ops are done, go to the reply state */
    if(io_ops_done_ == total_pipeline_ops_)
    {
        setNextMethod(&ReadTaskSM::postReply);
    }
}

void ReadTaskSM::getBMIBuffer()
{
    /* request a BMI buffer */
    bpool_->allocCB(slots_[READ_SLOT], request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND, rbuffer_[cw_post_index_]->buffer);

    /* set the callback and wait */
    slots_.wait(READ_SLOT, &ReadTaskSM::waitAllocateBMIBuffer);
}

void ReadTaskSM::sendPipelineBuffer()
{
    // The life cycle of buffers is like follows:
    // from alloc -> NetworkRecv -> rx_q -> ZoidI/O -> io_q -> back to alloc

    /* if there is still data to be recieved */
    //p_siz_ = std::min(bpool_->pipeline_size(), total_bytes_ - cur_sent_bytes_);
    request_.sendPipelineBufferCB(slots_[READ_SLOT], rbuffer_[cw_post_index_]->buffer->get_buf(), p_siz_);

    /* set the callback and wait */
    slots_.wait(READ_SLOT, &ReadTaskSM::waitSendPipelineBuffer);
}
    }
}
