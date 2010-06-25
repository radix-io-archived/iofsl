#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH

#include "Request.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwd/RetrievedBuffer.hh"
#include "iofwdutil/transform/IOFWDZLib.hh"

namespace iofwd
{
class WriteRequest : public Request
{
public:
  typedef struct ReqParam_
  {
     iofwdutil::iofwdtransform::GenericTransform *GenTransform;
     CBType UserCB;
     char *compressMem;
     int current_read_buf;
     size_t compressed_size;

     zoidfs::zoidfs_handle_t * handle;
     size_t mem_count;
     char ** mem_starts;
     size_t * mem_sizes;
     size_t mem_total_size;
     bmi_size_t mem_expected_size;
     bmi_size_t * bmi_mem_sizes;
     size_t file_count;
     zoidfs::zoidfs_file_ofs_t * file_starts;
     zoidfs::zoidfs_file_ofs_t * file_sizes;
     size_t pipeline_size;
     zoidfs::zoidfs_op_hint_t * op_hint;

     /* decoded hint values */
     bool op_hint_pipeline_enabled;
     bool op_hint_compress_enabled;

     size_t max_buffer_size;

     ReqParam_() : handle(NULL), mem_count(0), mem_starts(NULL), mem_sizes(NULL), mem_total_size(0), mem_expected_size(0),
        bmi_mem_sizes(NULL), file_count(0), file_starts(NULL), file_sizes(NULL), pipeline_size(0), op_hint(NULL), op_hint_pipeline_enabled(true), max_buffer_size(0)
     {
     }

  } ReqParam;

  WriteRequest (int opid)
     : Request (opid)
  {
  }

  virtual ReqParam & decodeParam () = 0;

  virtual void reply(const CBType & cb) = 0;

  // for normal mode
  virtual void recvBuffers(const CBType & cb, RetrievedBuffer * rb) = 0;
  virtual void recvComplete(int recvStatus) = 0;

  // for pipeline mode
  virtual void recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size) = 0;

  //virtual iofwdutil::bmi::BMIAddr getRequestAddr() = 0;

  virtual void initRequestParams(ReqParam & p, void * bufferMem) = 0;

  virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb) = 0;

  virtual void releaseBuffer(RetrievedBuffer * rb) = 0;
};

}

#endif
