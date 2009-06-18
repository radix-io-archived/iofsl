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
       mem_starts_(NULL), mem_sizes_(NULL),
       file_starts_(NULL), file_sizes_(NULL)
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
   uint32_t mem_count_;
   char ** mem_starts_;
   uint32_t * mem_sizes_;
   uint32_t file_count_;
   uint64_t * file_starts_;
   uint64_t * file_sizes_;
   uint64_t pipeline_size_;
}; 

//===========================================================================
   }
}

#endif
