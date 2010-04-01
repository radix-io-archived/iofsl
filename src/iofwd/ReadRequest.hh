#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdevent/CBType.hh"

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
   typedef struct ReqParam_
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

     ReqParam_() : handle(NULL), mem_count(0), mem_starts(NULL), mem_sizes(NULL), bmi_mem_sizes(NULL),
        file_count(0), mem_total_size(0), file_starts(NULL), file_sizes(NULL), pipeline_size(0), op_hint(NULL)
     {
     }
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
   virtual void sendPipelineBufferCB(iofwdevent::CBType cb, iofwdutil::bmi::BMIBuffer * buf, size_t size) = 0;

   virtual iofwdutil::bmi::BMIAddr getRequestAddr() = 0;
};

}

#endif
