#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH

#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

namespace iofwd
{
   
   /**
    * @TODO This is wrong.
    * The Read/Write Request cannot expose transport specific details.
    * They are here to hide those details from the tasks!
    * 
    *  virtual iofwdutil::bmi::BMIAddr getRequestAddr()
    *
    *  cannot be here.
    *
    *  @TODO Remove CompletionID * stuff
    */
class WriteRequest : public Request 
{
public:
  typedef struct
  {
     zoidfs::zoidfs_handle_t * handle;
     size_t mem_count;
     char ** mem_starts;
     size_t * mem_sizes;
     size_t file_count;
     zoidfs::zoidfs_file_ofs_t * file_starts;
     zoidfs::zoidfs_file_ofs_t * file_sizes;
     size_t pipeline_size;
     zoidfs::zoidfs_op_hint_t * op_hint;
  } ReqParam;

  WriteRequest (int opid)
     : Request (opid)
  {
  }

  virtual const ReqParam & decodeParam () = 0;

  virtual iofwdutil::completion::CompletionID * reply () = 0;

  // for normal mode
  virtual iofwdutil::completion::CompletionID * recvBuffers() = 0;

  // for pipeline mode
  virtual iofwdutil::completion::CompletionID * recvPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size) = 0;

  virtual iofwdutil::bmi::BMIAddr getRequestAddr() = 0;
}; 

}

#endif
