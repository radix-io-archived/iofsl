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
        /* cleanup rbuffer_ wrappers */
        for(int i = 0 ; i < total_pipeline_ops_ ; i++)
        {
            delete rbuffer_[i];
        }
        delete [] rbuffer_;

        /* delete the request last */
        delete &request_;
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
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
        sched_->enqueueWriteCB(slots_[WRITE_SLOT], p.handle, (size_t)p.mem_count, (const void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
#else
        sched_->enqueueWriteCB(slots_[WRITE_SLOT], p.handle, (size_t)p.mem_count, (const void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
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

    void WriteTaskSM::computePipelineFileSegments()
    {
        const zoidfs_file_ofs_t * file_starts = p.file_starts;
        const zoidfs_file_size_t * file_sizes = p.file_sizes;
        
        size_t cur_pipe_ofs = 0;
        int cur_file_index = 0;
        int num_pipe_segments = 0;
        size_t pipeline_size = bpool_->pipeline_size();
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

    void WriteTaskSM::execPipelineIO()
    {
        const char * p_buf = (char *)rbuffer_[cw_post_index_]->buffer->get_buf()->get();
        int p_seg_start = p_segments_start[cw_post_index_];
        int p_file_count = p_segments[cw_post_index_]; 
        int * ret = new int(0);
    
        /* setup segment data structures */
        zoidfs_file_ofs_t * file_starts = new zoidfs_file_ofs_t[p_file_count];
        zoidfs_file_size_t * file_sizes = new zoidfs_file_size_t[p_file_count];
        const char ** mem_starts = new const char*[p_file_count];
        size_t * mem_sizes = new size_t[p_file_count];

        for (int i = 0; i < p_file_count; i++)
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
        if((mode_ & WRITESM_SERIAL_IO_PIPELINE) == 0)
        {
            /* enqueue the write */
            sched_->enqueueWriteCB (
                slots_[WRITE_SLOT], p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
                file_starts, file_sizes, p.op_hint);

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
            file_starts, file_sizes, p.op_hint);


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
