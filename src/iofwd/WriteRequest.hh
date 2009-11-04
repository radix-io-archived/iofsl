#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH

#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
   
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
  virtual ~WriteRequest ()
  {
  }

  virtual const ReqParam & decodeParam () = 0;

  virtual iofwdutil::completion::CompletionID * reply () = 0;

  // for normal mode
  virtual iofwdutil::completion::CompletionID * recvBuffers() = 0;

  // for pipeline mode
  virtual iofwdutil::completion::CompletionID * recvPipelineBuffer(char *buf, size_t size) = 0;
}; 

}

#endif
