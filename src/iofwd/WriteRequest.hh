#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH

#include "Request.hh"

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
  } ReqParam;

  WriteRequest (int opid)
     : Request (opid)
  {
  }
  virtual ~WriteRequest ()
  {
  }

  virtual const ReqParam & decodeParam () = 0;

  virtual iofwdutil::completion::CompletionID * recvBuffers() = 0;
  virtual iofwdutil::completion::CompletionID * reply () = 0;
}; 

}

#endif
