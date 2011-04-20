#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH


#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidFSOpHint.hh"

#include "iofwd/Request.hh"
#include "iofwd/RetrievedBuffer.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>


namespace iofwd
{
class WriteRequest : public Request
{
public:
  typedef struct ReqParam_
  {
      zoidfs::zoidfs_handle_t * handle;
      
      size_t mem_count;
      boost::scoped_array<char *> mem_starts;
      boost::scoped_array<size_t> mem_sizes;
      size_t mem_total_size;
     
      size_t file_count;
      boost::scoped_array<zoidfs::zoidfs_file_ofs_t> file_starts;
      boost::scoped_array<zoidfs::zoidfs_file_ofs_t> file_sizes;
      
      size_t pipeline_size;
      bool op_hint_pipeline_enabled;
      
      zoidfs::util::ZoidFSOpHint * op_hint;

      size_t max_buffer_size;
      
      ReqParam_() :
          handle(NULL), 
          mem_count(0),
          mem_total_size(0),
          file_count(0),
          pipeline_size(0),
          op_hint_pipeline_enabled(true),
          op_hint(NULL)
      {
      }

      ~ReqParam_()
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

  // for pipeline mode
  virtual void recvPipelineBufferCB(iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size) = 0;

  virtual void initRequestParams(ReqParam & p, void * bufferMem) = 0;

  virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb) = 0;

  virtual void releaseBuffer(RetrievedBuffer * rb) = 0;
};

}

#endif
