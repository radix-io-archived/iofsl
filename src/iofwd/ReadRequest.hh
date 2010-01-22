#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

namespace iofwd
{
   /**
    * @TODO This is wrong.
    * The Read/Write Request cannot expose transport specific details.
    * They are here to hide those details from the tasks!
    * Solution: cast the interface in terms of functionality and not
    * transport specific things.
    * There cannot be a getRequestAddr() function.
    */
class ReadRequest : public Request
{
public:
   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      size_t mem_count;
      char ** mem_starts;
      size_t * mem_sizes;
      bmi_size_t * bmi_mem_sizes;
      size_t file_count;
      bmi_size_t mem_total_size;
      zoidfs::zoidfs_file_ofs_t * file_starts;
      zoidfs::zoidfs_file_ofs_t * file_sizes;
      size_t pipeline_size;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   ReadRequest (int opid)
      : Request (opid)
   {
   }


   virtual const ReqParam & decodeParam () = 0;

   virtual void reply(const CBType & cb) = 0;

   // for normal mode
   virtual void sendBuffers(const CBType & cb) = 0;

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * sendPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size) = 0;

   virtual iofwdutil::bmi::BMIAddr getRequestAddr() = 0;
};

}

#endif
