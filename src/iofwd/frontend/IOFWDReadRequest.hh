#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class  IOFWDReadRequest 
   : public IOFWDRequest, 
     public ReadRequest
{
public:
   IOFWDReadRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), ReadRequest (opid)
   {
   }
   virtual ~IOFWDReadRequest ();

   virtual const ReqParam & decodeParam ();

   virtual iofwdutil::completion::CompletionID * reply();

   // for normal mode
   virtual iofwdutil::completion::CompletionID * sendBuffers ();

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * sendPipelineBuffer(char * buf, size_t size);

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
