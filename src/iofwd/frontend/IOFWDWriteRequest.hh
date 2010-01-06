#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDWriteRequest 
   : public IOFWDRequest, 
     public WriteRequest
{
public:
   IOFWDWriteRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
     : IOFWDRequest (bmi, info,res), WriteRequest (opid),
       mem_count_ (0), mem_total_size_(0), mem_ (NULL), mem_starts_(NULL), mem_sizes_(NULL), bmi_mem_sizes_(NULL),
       file_count_ (0), file_starts_(NULL), file_sizes_(NULL),
       pipeline_size_ (0), op_hint_(NULL)
   {
   }
   virtual ~IOFWDWriteRequest ();

   virtual const ReqParam & decodeParam ();

   virtual iofwdutil::completion::CompletionID * reply();

   // for normal mode
   virtual iofwdutil::completion::CompletionID * recvBuffers();

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * recvPipelineBuffer(char *buf, size_t size);

private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   size_t mem_count_;
   size_t mem_total_size_;
   char * mem_;
   char ** mem_starts_;
   size_t * mem_sizes_;
   bmi_size_t * bmi_mem_sizes_;
   size_t file_count_;
   zoidfs::zoidfs_file_ofs_t * file_starts_;
   zoidfs::zoidfs_file_ofs_t * file_sizes_;
   size_t pipeline_size_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
}; 

//===========================================================================
   }
}

#endif
