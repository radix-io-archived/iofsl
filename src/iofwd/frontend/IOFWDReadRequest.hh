#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwd/BMIBufferPool.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

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
      : IOFWDRequest (bmi, info,res), ReadRequest (opid),
        mem_count_ (0), mem_total_size_(0), mem_ (NULL), bmi_buffer_(addr_, iofwdutil::bmi::BMI::ALLOC_SEND), mem_starts_ (NULL), mem_sizes_ (NULL), bmi_mem_sizes_ (NULL),
        file_count_ (0), file_starts_ (NULL), file_sizes_ (NULL),
        pipeline_size_ (0), op_hint_(NULL)
   {
   }
   virtual ~IOFWDReadRequest ();

   virtual const ReqParam & decodeParam ();

   virtual iofwdutil::completion::CompletionID * reply();

   // for normal mode
   virtual iofwdutil::completion::CompletionID * sendBuffers ();

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * sendPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size);

   virtual iofwdutil::bmi::BMIAddr getRequestAddr()
   {
      return addr_;
   }

private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   size_t mem_count_;
   size_t mem_total_size_;
   char * mem_;
   iofwdutil::bmi::BMIBuffer bmi_buffer_;
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
