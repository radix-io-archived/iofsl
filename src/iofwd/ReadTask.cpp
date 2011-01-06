#include "ReadTask.hh"
#include "zoidfs/zoidfs-proto.h"

#include <vector>
#include <deque>

using namespace std;

namespace iofwd
{
//===========================================================================

void ReadTask::runNormalMode(ReadRequest::ReqParam & p)
{
   int ret;

   // get a buffer
   block_.reset();
   request_.allocateBuffer((block_), rbuffer_[0]);
   block_.wait();

   // init the task params
   request_.initRequestParams(p, rbuffer_[0]->getptr());

   // issue the read request
   block_.reset();
   api_->read(block_, &ret, p.handle, p.mem_count, p.mem_starts,
         p.mem_sizes, p.file_count, p.file_starts, p.file_sizes, p.op_hint);
   block_.wait();

   request_.setReturnCode(ret);

   // send buffers w/ callback
   block_.reset();
   request_.sendBuffers((block_), rbuffer_[0]);
   block_.wait();

   /* deallocate the buffer after the send */
   request_.releaseBuffer(rbuffer_[0]);

   // send reply w/ callback
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

void ReadTask::computePipelineFileSegments(const ReadRequest::ReqParam & p)
{
    const zoidfs::zoidfs_file_ofs_t * file_starts = p.file_starts;
    const zoidfs::zoidfs_file_size_t * file_sizes = p.file_sizes;

    size_t cur_pipe_ofs = 0;
    int cur_file_index = 0;
    int num_pipe_segments = 0;
    size_t cur_pipe_buffer_size = pipeline_size_;
    zoidfs::zoidfs_file_size_t cur_file_size = file_sizes[cur_file_index];
    zoidfs::zoidfs_file_ofs_t cur_file_start = file_starts[cur_file_index];
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

void ReadTask::sendPipelineBuffer(int index)
{
    /* if there is still data to be recieved */
    block_.reset();
    p_siz_ = std::min(pipeline_size_, total_bytes_ - cur_sent_bytes_);
    request_.sendPipelineBufferCB(block_, rbuffer_[index], p_siz_);

    /* set the callback and wait */
    block_.wait();

    /* dealloc the buffer */
    request_.releaseBuffer(rbuffer_[index]);

    /* update the byte transfer count */
    cur_sent_bytes_ += p_siz_;
}

void ReadTask::execPipelineIO(const ReadRequest::ReqParam & p)
{
    int index = 0;
    int pipe_read_ops_posted = total_pipeline_ops_;
    int pipe_read_ops_done = total_pipeline_ops_;
    int pipe_sent_ops = total_pipeline_ops_;
    int pipe_buffer_ops = total_pipeline_ops_;
    int cur_send_op = 0;
    bool * send_ready = new bool[total_pipeline_ops_];

    for(int i = 0 ; i < total_pipeline_ops_ ; i++)
    {
        send_ready[i] = false;
    }

    /* while the pipeline operation is not done... */
    while( pipe_read_ops_posted > 0 || pipe_read_ops_done > 0 || pipe_buffer_ops > 0 )
    {
        /* get a buffer */
        if(pipe_buffer_ops > 0)
        {
            block_.reset();
            request_.allocateBuffer(block_, rbuffer_[index]);
            block_.wait();
            pipe_buffer_ops--;
        }

        /* post a read op */
        if(pipe_read_ops_posted > 0)
        {
            postRead(p, index);
            pipe_read_ops_posted--;

            /* add the pipeline op to the lookup vector */
            pipeline_ops_.push_back(index);

            /* advance to the next pipeline op */
            index++;
        }

        /* check and see if any pending reads completed */
        if(pipe_read_ops_done > 0)
        {
            /* iterate over the pipeline op vector */
            unsigned int i = 0;
            while(i < pipeline_ops_.size())
            {
                /* get the index we are going to test */
                int cur = pipeline_ops_[i];

                /* if this block completed */
                if(pipeline_blocks_[cur]->test())
                {
                    /* reset the pipeline block */
                    pipeline_blocks_[cur]->wait();

                    /* update the count */
                    pipe_read_ops_done--;

                    send_ready[cur] = true;

                    /* remove the completed op from the pipeline op list */
                    pipeline_ops_.erase(pipeline_ops_.begin() + i);
                }
                else
                {
                    /* update the counter */
                    i++;
                }
            }

            /* send any data that is ready */
            bool sendMoreData = true;
            do
            {
                if(cur_send_op < total_pipeline_ops_ && send_ready[cur_send_op])
                {
                    sendPipelineBuffer(cur_send_op);
                    cur_send_op++;
                    pipe_sent_ops--;
                }
                else
                {
                    sendMoreData = false;
                }
            }while(sendMoreData);
        }
    }

    /* check for completed read ops after all of the ops have been posted */
    while(pipe_read_ops_done > 0)
    {
        /* iterate over the pipeline op vector */
        unsigned int i = 0;
        while(i < pipeline_ops_.size())
        {
            /* get the index we are going to test */
            int cur = pipeline_ops_[i];

            /* if this block completed */
            if(pipeline_blocks_[cur]->test())
            {
                /* reset the pipeline block */
                pipeline_blocks_[cur]->reset();

                /* update the count */
                pipe_read_ops_done--;

                send_ready[cur] = true;

                /* remove the completed op from the pipeline op list */
                pipeline_ops_.erase(pipeline_ops_.begin() + i);
            }
            else
            {
                /* update the counter */
                i++;
            }
        }

        /* send any data that is ready */
        bool sendMoreData = true;
        do
        {
            if(cur_send_op < total_pipeline_ops_ && send_ready[cur_send_op])
            {
                sendPipelineBuffer(cur_send_op);
                cur_send_op++;
                pipe_sent_ops--;
            }
            else
            {
                sendMoreData = false;
            }
        }while(sendMoreData);
    }

    /* send the data to the client */
    while(pipe_sent_ops > 0)
    { 
        /* sent data into a buffer */
        sendPipelineBuffer(cur_send_op);
        pipe_sent_ops--;
        cur_send_op++;
    }

    /* delete the ready flags */
    delete [] send_ready;
}

void ReadTask::runPostReadCB(iofwdevent::CBException status,
      int index, iofwdevent::CBType cb)
{
    /* update the ret code */
    if(*(rbuffer_[index]->ret) != zoidfs::ZFS_OK)
    {
        ret_ = *(rbuffer_[index]->ret);
    }

    cb(status);
}

void ReadTask::postRead(const ReadRequest::ReqParam & p, int index)
{
    char * p_buf = (char *)rbuffer_[index]->buffer_->getMemory();
    int p_seg_start = p_segments_start[index];
    int p_file_count = p_segments[index];
    int * ret = new int(0);

    /* setup segment data structures */
    zoidfs::zoidfs_file_ofs_t * file_starts = new zoidfs::zoidfs_file_ofs_t[p_file_count];
    zoidfs::zoidfs_file_size_t * file_sizes = new zoidfs::zoidfs_file_size_t[p_file_count];
    char ** mem_starts = new char*[p_file_count];
    size_t * mem_sizes = new size_t[p_file_count];

    // Use helper class from cache tree to iterator through mem/file ranges.
    // @TODO: This code assumes file_count == mem_count
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

    rbuffer_[index]->mem_starts = const_cast<const char**>(mem_starts);
    rbuffer_[index]->mem_sizes = mem_sizes;
    rbuffer_[index]->file_starts = file_starts;
    rbuffer_[index]->file_sizes = file_sizes;
    rbuffer_[index]->ret = ret;

    iofwdevent::CBType pcb = *(pipeline_blocks_[index]);
    iofwdevent::CBType bmmCB = boost::bind(&iofwd::ReadTask::runPostReadCB,
            this, _1, index, pcb);

    /* enqueue the read */
    api_->read (
        bmmCB, ret, p.handle, p_file_count,
        reinterpret_cast<void**>(mem_starts), mem_sizes, p_file_count, file_starts,
        file_sizes, const_cast<zoidfs::zoidfs_op_hint_t *>(p.op_hint));
}

void ReadTask::runPipelineMode(const ReadRequest::ReqParam & p)
{
   // run the pipeline IO transfer code
   execPipelineIO(p);

   // reply status
   request_.setReturnCode(ret_);
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

//===========================================================================
}
