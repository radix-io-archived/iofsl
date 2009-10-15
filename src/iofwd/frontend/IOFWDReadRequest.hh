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
      : IOFWDRequest (bmi, info,res), ReadRequest (opid),
        mem_count_ (0), mem_ (NULL), mem_starts_ (NULL), mem_sizes_ (NULL),
        file_count_ (0), file_starts_ (NULL), file_sizes_ (NULL),
        pipeline_size_ (0)
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
   size_t mem_count_;
   char * mem_;
   char ** mem_starts_;
   size_t * mem_sizes_;
   size_t file_count_;
   zoidfs::zoidfs_file_ofs_t * file_starts_;
   zoidfs::zoidfs_file_ofs_t * file_sizes_;
   size_t pipeline_size_;
}; 

//===========================================================================
   }
}

#endif
