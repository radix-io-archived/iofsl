#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
   
class ReadRequest : public Request 
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
   } ReqParam;

   ReadRequest (int opid)
      : Request (opid)
   {
   }
   virtual ~ReadRequest ()
   {
   }


   virtual const ReqParam & decodeParam () = 0;

   virtual iofwdutil::completion::CompletionID * reply () = 0;

   // for normal mode
   virtual iofwdutil::completion::CompletionID * sendBuffers () = 0;

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * sendPipelineBuffer(char * buf, size_t size) = 0;
};

}

#endif
