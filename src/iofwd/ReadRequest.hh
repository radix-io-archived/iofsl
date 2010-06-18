#ifndef IOFWD_READREQUEST_HH
#define IOFWD_READREQUEST_HH

#include <cstring>
#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdevent/CBType.hh"
#include "iofwd/RetrievedBuffer.hh"

namespace iofwd
{
class ReadRequest : public Request
{
public:
   typedef struct ReqParam_
   {
      zoidfs::zoidfs_handle_t * handle;
      size_t mem_count;
      void ** mem_starts;
      size_t * mem_sizes;
      bmi_size_t * bmi_mem_sizes;
      size_t file_count;
      bmi_size_t mem_total_size;
      zoidfs::zoidfs_file_ofs_t * file_starts;
      zoidfs::zoidfs_file_ofs_t * file_sizes;
      size_t pipeline_size;
      zoidfs::zoidfs_op_hint_t * op_hint;

      /* decoded hint values */
     bool op_hint_pipeline_enabled;

     size_t max_buffer_size;

     ReqParam_() : handle(NULL), mem_count(0), mem_starts(NULL), mem_sizes(NULL), bmi_mem_sizes(NULL),
        file_count(0), mem_total_size(0), file_starts(NULL), file_sizes(NULL), pipeline_size(0), op_hint(NULL), op_hint_pipeline_enabled(true), max_buffer_size(0)
     {
     }
   } ReqParam;

   ReadRequest (int opid)
      : Request (opid)
   {
   }


   virtual ReqParam & decodeParam () = 0;

   virtual void reply(const CBType & cb) = 0;

   // for normal mode
   virtual void sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb) = 0;

   // for pipeline mode
   virtual void sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size) = 0;

   virtual void initRequestParams(ReqParam & p, void * bufferMem) = 0;

   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb) = 0;

   virtual void releaseBuffer(RetrievedBuffer * rb) = 0;
};

}

#endif
