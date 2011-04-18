#include "iofwd/tasksm/WriteTaskSM.hh"
#include "zoidfs/zoidfs-proto.h"

namespace iofwd
{
    namespace tasksm
    {
//===========================================================================

    WriteTaskSM::WriteTaskSM(sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * p)
        : 
            sm::SimpleSM<WriteTaskSM>(smm),
            api_(api),
            request_((static_cast<WriteRequest&>(*p))),
            p(request_.decodeParam()),
            slots_(*this),
            total_bytes_(0),
            cur_recv_bytes_(0),
            p_siz_(0),
            total_pipeline_ops_(0),
            total_buffers_(0),
            io_ops_done_(0),
            cw_post_index_(0),
            rbuffer_(NULL),
            mode_(WRITESM_PARA_IO_PIPELINE),
            ret_(zoidfs::ZFS_OK),
            pipeline_size_(0)
    {
    }

    WriteTaskSM::~WriteTaskSM()
    {
        /* cleanup rbuffer_ wrappers */
        for(int i = 0 ; i < total_buffers_ ; i++)
        {
            delete rbuffer_[i];
        }

        if(rbuffer_)
        {
            delete [] rbuffer_;
        }

        /* delete the request last */
        delete &request_;
    }

    /* recv the input data to write to the disk */
    void WriteTaskSM::recvBuffers()
    {
        /* issue the recv buffer */
        request_.recvBuffers(slots_[WRITE_SLOT], rbuffer_[0]);

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitRecvInputBuffers);
    }

    /* execute the write I/O path */
    void WriteTaskSM::writeNormal()
    {
#if SIZEOF_SIZE_T == SIZEOF_INT64_T
        api_->write(slots_[WRITE_SLOT], &ret_, p.handle, p.mem_count,
              const_cast<const void**>(reinterpret_cast<void**>(p.mem_starts.get())), p.mem_sizes.get(),
              p.file_count, p.file_starts.get(), p.file_sizes.get(),
              const_cast<zoidfs::zoidfs_op_hint_t *>(p.op_hint));
#else
        api_->write(slots_[WRITE_SLOT], &ret_, p.handle, p.mem_count,
              const_cast<const void**>(reinterpret_cast<void**>(p.mem_starts.get())), p.mem_sizes.get(), p.file_count, p.file_starts.get(),
              p.file_sizes.get(), const_cast<zoidfs::zoidfs_op_hint_t *>(p.op_hint));
#endif

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitEnqueueWrite);
    }

    /* issue the reply */
    void WriteTaskSM::reply()
    {
        /* set the return code */
        request_.setReturnCode(ret_);

        /* issue the reply */
        request_.reply(slots_[WRITE_SLOT]);

        /* set the callback */
        slots_.wait(WRITE_SLOT, &WriteTaskSM::waitReply);
    }

    void WriteTaskSM::computePipelineFileSegments()
    {
        const zoidfs_file_ofs_t * file_starts = p.file_starts.get();
        const zoidfs_file_size_t * file_sizes = p.file_sizes.get();
        
        size_t cur_pipe_ofs = 0;
        int cur_file_index = 0;
        int num_pipe_segments = 0;
        size_t cur_pipe_buffer_size = pipeline_size_;
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
                    cur_pipe_buffer_size = pipeline_size_;
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
    }

    void WriteTaskSM::execPipelineIO()
    {
        const char * p_buf = (char *)rbuffer_[cw_post_index_]->buffer_->getMemory();
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
        if(mode_ == WRITESM_SERIAL_IO_PIPELINE)
        {
            /* enqueue the write */
            api_->write (
                slots_[WRITE_SLOT], ret, p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
                p_file_count, file_starts, file_sizes, const_cast<zoidfs::zoidfs_op_hint_t *>(p.op_hint));

            /* issue the serial wait */
            slots_.wait(WRITE_SLOT, &WriteTaskSM::waitPipelineEnqueueWrite);
        }
        else
        {
            int my_slot = 0;
            {
                boost::mutex::scoped_lock l(slot_mutex_);

                /* assign the current slot */
                my_slot = cw_post_index_;
                cw_post_index_++;

                /* update the byte transfer count */
                cur_recv_bytes_ += p_siz_;

                /* enqueue the write */
                iofwdevent::CBType barrierCB =
                   boost::bind(&iofwd::tasksm::WriteTaskSM::writeDoneCB, this,
                         _1, my_slot);
                api_->write (
                    barrierCB, ret, p.handle, p_file_count, (const
                       void**)mem_starts, mem_sizes, p_file_count,
                    file_starts, file_sizes,
                    const_cast<zoidfs::zoidfs_op_hint_t *>(p.op_hint));

                if(cur_recv_bytes_ == total_bytes_)
                {
                    s_ = slots_[WRITE_SLOT];
                    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitWriteBarrier);
                }
                else
                {
                    setNextMethod(&WriteTaskSM::waitPipelineEnqueueWrite);
                }
            }
        }
    }

void WriteTaskSM::writeDoneCB(iofwdevent::CBException status, int my_slot)
{
   status.check ();
    int count = 0;
    {
        boost::mutex::scoped_lock l(slot_mutex_);

        // update the op count
        io_ops_done_++;
        count = io_ops_done_;

        /* update the return code */
        if(*(rbuffer_[my_slot]->ret) != zoidfs::ZFS_OK)
        {
            ret_ = *(rbuffer_[my_slot]->ret);
        }

        /* free the buffer */
        request_.releaseBuffer(rbuffer_[my_slot]);
    }

    if(count == total_pipeline_ops_)
    {
        s_(status);
    }
}

/* barrier for writes... will not go to post reply state until all writes complete */
void WriteTaskSM::waitWriteBarrier(iofwdevent::CBException e)
{
   e.check ();
    int count = 0;
    {
        boost::mutex::scoped_lock l(slot_mutex_);
        count = io_ops_done_;
    }
    if(count == total_pipeline_ops_)
    {
        setNextMethod(&WriteTaskSM::postReply);
    }
}

void WriteTaskSM::getBuffer()
{
    /* request a buffer */
    request_.allocateBuffer(slots_[WRITE_SLOT], rbuffer_[cw_post_index_]);

    /* set the callback and wait */
    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitAllocateBuffer);
}

void WriteTaskSM::getSingleBuffer()
{
    /* request a buffer */
    request_.allocateBuffer(slots_[WRITE_SLOT], rbuffer_[0]);

    /* set the callback and wait */
    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitAllocateSingleBuffer);
}

void WriteTaskSM::recvPipelineBuffer()
{
    /* if there is still data to be recieved */
    p_siz_ = std::min(pipeline_size_, total_bytes_ - cur_recv_bytes_);
    request_.recvPipelineBufferCB(slots_[WRITE_SLOT], rbuffer_[cw_post_index_], p_siz_);

    /* set the callback and wait */
    slots_.wait(WRITE_SLOT, &WriteTaskSM::waitRecvPipelineBuffer);
}

//===========================================================================
    }
}
