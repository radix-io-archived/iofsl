#ifndef IOFWD_READTASK_HH
#define IOFWD_READTASK_HH

#include <vector>
#include "iofwd/Task.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwd/TaskHelper.hh"
#include <boost/function.hpp>
#include "iofwdutil/InjectPool.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwd/RetrievedBuffer.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/BMIMemoryManager.hh"

namespace iofwd
{

class ReadTask : public TaskHelper<ReadRequest>, public iofwdutil::InjectPool<ReadTask>
{
public:
   ReadTask (ThreadTaskParam & p)
      : TaskHelper<ReadRequest>(p), total_bytes_(0), cur_sent_bytes_(0), p_siz_(0), total_pipeline_ops_(0),
        total_buffers_(0), rbuffer_(NULL), pipeline_blocks_(NULL)
   {
   }

   virtual ~ReadTask()
   {
        /* cleanup rbuffer_ wrappers */
        for(int i = 0 ; i < total_buffers_ ; i++)
        {
            delete rbuffer_[i];
        }
        delete [] rbuffer_;

        /* cleanup up the pipeline blocks */
        for(int i = 0 ; i < total_pipeline_ops_ ; i++)
        {
            pipeline_blocks_[i]->reset();
            delete pipeline_blocks_[i];
        }

        /* only delete the pipeline blocks if this was a pipeline op */
        if(total_pipeline_ops_ > 0)
        {
            delete [] pipeline_blocks_;
        }
   }

   void run ()
   {
      // parameter decode
      ReadRequest::ReqParam & p = request_.decodeParam();

      /* compute the total size of the transfers */
      for (size_t i = 0; i < p.mem_count; i++)
          total_bytes_ += p.mem_sizes[i];

      if (p.pipeline_size == 0)
      {
         /* setup the rbuffer variable */
         rbuffer_ = new RetrievedBuffer*[1];
         rbuffer_[0] = new RetrievedBuffer(request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND, total_bytes_);
         rbuffer_[0]->reinit();
         total_buffers_ = 1;
         runNormalMode(p);
      }
      else
      {
            /* compute the total number of concurrent pipeline ops */
            total_pipeline_ops_ = (int)ceil(1.0 * total_bytes_ / iofwd::BMIMemoryManager::instance().pipeline_size());
            total_buffers_ = total_pipeline_ops_;

            computePipelineFileSegments(p);

            /* setup the rbuffer variable */
            bool lastBufferIsPartial = (total_bytes_ % iofwd::BMIMemoryManager::instance().pipeline_size() == 0 ? false : true);
            rbuffer_ = new RetrievedBuffer*[total_pipeline_ops_];
            for(int i = 0 ; i < total_pipeline_ops_ ; i++)
            {
                if(lastBufferIsPartial && i + 1 == total_pipeline_ops_)
                {
                    rbuffer_[i] = new RetrievedBuffer(request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND, total_bytes_ % iofwd::BMIMemoryManager::instance().pipeline_size());
                }
                else
                {
                    rbuffer_[i] = new RetrievedBuffer(request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND, iofwd::BMIMemoryManager::instance().pipeline_size());
                }
                rbuffer_[i]->reinit();
            }

            /* setup the pipeline blocks */
            pipeline_blocks_ = new iofwdevent::SingleCompletion*[total_pipeline_ops_];
            for(int i = 0 ; i < total_pipeline_ops_ ; i++)
            {
                pipeline_blocks_[i] = new iofwdevent::SingleCompletion();
            }

            /* run the pipeline code */
            runPipelineMode(p);
      }
   }

private:
   void runNormalMode(ReadRequest::ReqParam & p);
   void runPipelineMode(const ReadRequest::ReqParam & p);
   void execPipelineIO(const ReadRequest::ReqParam & p);
   void postRead(const ReadRequest::ReqParam & p, int index);
   void getBMIBuffer(int index);
   void computePipelineFileSegments(const ReadRequest::ReqParam & p);
   void sendPipelineBuffer(int index);

   /* pipeline variables */
   size_t total_bytes_;
   size_t cur_sent_bytes_;
   size_t p_siz_;
   int total_pipeline_ops_;
   int total_buffers_;
   RetrievedBuffer ** rbuffer_;
   iofwdevent::SingleCompletion ** pipeline_blocks_;

   /* @TODO: we only need these vectors when we pipeline... maybe we need seperate pipeline and normal tasks
      so that we can save mem and alloc time when we are in normal mode ? */
   std::vector<zoidfs::zoidfs_file_size_t> p_file_sizes;
   std::vector<zoidfs::zoidfs_file_ofs_t> p_file_starts;
   std::vector<size_t> p_mem_offsets;
   std::vector<int> p_segments;
   std::vector<int> p_segments_start;
   std::vector<int> pipeline_ops_;
};

}

#endif
