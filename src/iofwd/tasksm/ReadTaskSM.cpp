#include "iofwd/tasksm/ReadTaskSM.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd/RequestScheduler.hh"

namespace iofwd
{
    namespace tasksm
    {

    ReadTaskSM::ReadTaskSM(sm::SMManager & smm, RequestScheduler * sched, Request * p)
        : sm::SimpleSM<ReadTaskSM>(smm), sched_(sched), request_((static_cast<ReadRequest&>(*p))), slots_(*this),
          total_bytes_(0), cur_sent_bytes_(0), p_siz_(0), total_pipeline_ops_(0), total_buffers_(0),
          io_ops_done_(0), cw_post_index_(0), rbuffer_(NULL), mode_(READSM_SERIAL_IO_PIPELINE)
    {
    }

    ReadTaskSM::~ReadTaskSM()
    {
        /* cleanup rbuffer_ wrappers */
        for(int i = 0 ; i < total_buffers_ ; i++)
        {
            delete rbuffer_[i];
        }
        if(rbuffer_)
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
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
        sched_->enqueueReadCB(slots_[READ_SLOT], p.handle, (size_t)p.mem_count, (void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
#else
        sched_->enqueueReadCB(slots_[READ_SLOT], p.handle, (size_t)p.mem_count, (void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
#endif

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

    void ReadTaskSM::computePipelineFileSegments()
    {
        const zoidfs_file_ofs_t * file_starts = p.file_starts;
        const zoidfs_file_size_t * file_sizes = p.file_sizes;

        size_t cur_pipe_ofs = 0;
        int cur_file_index = 0;
        int num_pipe_segments = 0;
        size_t pipeline_size = iofwd::BMIMemoryManager::instance().pipeline_size();
        size_t cur_pipe_buffer_size = pipeline_size;
        zoidfs_file_size_t cur_file_size = file_sizes[cur_file_index];
        zoidfs_file_ofs_t cur_file_start = file_starts[cur_file_index];
        size_t cur_mem_offset = 0;
        int cur_pipe = 0;

        /* while there is still data to process in this op */
        p_segments.push_back(0);
        p_segments_start.push_back(0);
        while(cur_pipe_ofs < total_bytes_)
        {
            /* setup the file sizes */
            cur_file_size = file_sizes[cur_file_index];
            cur_file_start = file_starts[cur_file_index];

            while(cur_file_size > 0)
            {
                /* if the data left in the current file is large than the remaining pipeline buffer */
                if(cur_file_size > cur_pipe_buffer_size)
                {
                    /* update the pipeline segment data */
                    p_file_sizes.push_back(cur_pipe_buffer_size);
                    p_file_starts.push_back(cur_file_start);
                    p_mem_offsets.push_back(cur_mem_offset);
                    p_segments[cur_pipe] += 1;

                    cur_file_start += cur_pipe_buffer_size;

                    cur_file_size -= cur_pipe_buffer_size;
                    cur_pipe_ofs += cur_pipe_buffer_size;
                    cur_pipe_buffer_size = pipeline_size;
                    cur_pipe++;
                    p_segments.push_back(0);
                    cur_mem_offset = 0;
                    num_pipe_segments++;
                    p_segments_start.push_back(num_pipe_segments);
                }
                /* buffer consumes the rest of the segment */
                else
                {
                    /* update the pipeline segment data */
                    p_file_sizes.push_back(cur_file_size);
                    p_file_starts.push_back(cur_file_start);
                    p_mem_offsets.push_back(cur_mem_offset);
                    p_segments[cur_pipe] += 1;

                    cur_file_start += cur_file_size;

                    cur_pipe_buffer_size -= cur_file_size;
                    cur_pipe_ofs += cur_file_size;
                    cur_mem_offset += cur_file_size;
                    cur_file_size = 0;
                    cur_file_index++;
                    num_pipe_segments++;
                }
            }
        }
#if 0
        for(int i = 0 ; i < p_segments.size() ; i++)
        {
            fprintf(stderr, "pipe transfer %i: # segments %llu\n", i, p_segments[i]);
        }
        for(int i = 0 ; i < p_file_starts.size() ; i++)
        {
            fprintf(stderr, "pipe transfer %i: file_starts = %llu, file_sizes = %llu, mem_offsets = %llu\n", i, p_file_starts[i], p_file_sizes[i], p_mem_offsets[i]);
        }
#endif
    }

    void ReadTaskSM::execPipelineIO()
    {
        const char * p_buf = (char *)rbuffer_[cw_post_index_]->buffer->getMemory();
        int p_seg_start = p_segments_start[cw_post_index_];
        size_t p_file_count = p_segments[cw_post_index_];
        int * ret = new int(0);

        /* setup segment data structures */
        zoidfs_file_ofs_t * file_starts = new zoidfs_file_ofs_t[p_file_count];
        zoidfs_file_size_t * file_sizes = new zoidfs_file_size_t[p_file_count];
        const char ** mem_starts = new const char*[p_file_count];
        size_t * mem_sizes = new size_t[p_file_count];

        for (size_t i = 0; i < p_file_count; i++)
        {
            /* find the index into the pipeline data */
            int p_index = p_seg_start + i;

            /* compute the I/O offsets for this data */
            mem_starts[i] = p_buf + p_mem_offsets[p_index];
            mem_sizes[i] = p_file_sizes[p_index];
            file_starts[i] = p_file_starts[p_index];
            file_sizes[i] = p_file_sizes[p_index];
        }

        rbuffer_[cw_post_index_]->mem_starts = mem_starts;
        rbuffer_[cw_post_index_]->mem_sizes = mem_sizes;
        rbuffer_[cw_post_index_]->file_starts = file_starts;
        rbuffer_[cw_post_index_]->file_sizes = file_sizes;
        rbuffer_[cw_post_index_]->ret = ret;

    /* set the callback and wait */
    if((mode_ & READSM_SERIAL_IO_PIPELINE) == 0)
    {
        /* enqueue the write */
        sched_->enqueueReadCB (
            slots_[READ_SLOT], p.handle, p_file_count, (void**)mem_starts, mem_sizes,
            file_starts, file_sizes, p.op_hint);

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
            file_starts, file_sizes, p.op_hint);


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
    iofwd::BMIMemoryManager::instance().alloc(slots_[READ_SLOT], rbuffer_[cw_post_index_]->buffer);

    /* set the callback and wait */
    slots_.wait(READ_SLOT, &ReadTaskSM::waitAllocateBMIBuffer);
}

void ReadTaskSM::getSingleBMIBuffer()
{
    /* request a BMI buffer */
    iofwd::BMIMemoryManager::instance().alloc(slots_[READ_SLOT], rbuffer_[0]->buffer);

    /* set the callback and wait */
    slots_.wait(READ_SLOT, &ReadTaskSM::waitAllocateSingleBuffer);
}

void ReadTaskSM::sendPipelineBuffer()
{
    // The life cycle of buffers is like follows:
    // from alloc -> NetworkRecv -> rx_q -> ZoidI/O -> io_q -> back to alloc

    /* if there is still data to be recieved */
    request_.sendPipelineBufferCB(slots_[READ_SLOT], rbuffer_[cw_post_index_]->buffer->getBMIBuffer(), p_siz_);

    /* set the callback and wait */
    slots_.wait(READ_SLOT, &ReadTaskSM::waitSendPipelineBuffer);
}
    }
}
