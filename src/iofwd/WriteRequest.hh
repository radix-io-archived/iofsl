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
     uint32_t mem_count;
     char ** mem_starts;
     uint32_t * mem_sizes;
     uint32_t file_count;
     uint64_t * file_starts;
     uint64_t * file_sizes;
     uint64_t pipeline_size;
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
