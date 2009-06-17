#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
   
class ReadRequest : public Request 
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

   ReadRequest (int opid)
      : Request (opid)
   {
   }
   virtual ~ReadRequest ()
   {
   }

   virtual const ReqParam & decodeParam () = 0;
   virtual iofwdutil::completion::CompletionID * replyBuffers () = 0;
};

}

#endif
