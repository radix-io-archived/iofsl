#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwdutil/HybridAllocator.hh"

#include <boost/scoped_ptr.hpp>
#include "src/iofwdutil/transform/IOFWDZLib.hh"


using namespace iofwdutil;

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class  IOFWDReadRequest
   : public IOFWDRequest,
     public ReadRequest,
     public iofwdutil::InjectPool<IOFWDReadRequest>
{
  protected:
     // shared state
     boost::scoped_ptr<iofwdutil::transform::GenericTransform> transform_;
     bool op_hint_compress_enabled_;

     // pipelined state
     char   **compressed_mem_;
     size_t compressed_size_;
     size_t rem_bytes_;
     bool   partial_slot_;
     size_t next_slot_;
     size_t user_callbacks_;
     size_t pipeline_ops_;
     boost::mutex mp_;

public:
   IOFWDReadRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), ReadRequest (opid),
        transform_(NULL),
        op_hint_compress_enabled_(false),
	compressed_mem_(NULL),
	compressed_size_(0),
	rem_bytes_(0),
	partial_slot_(false),
        next_slot_(0),
	user_callbacks_(0),
	pipeline_ops_(0)
   {
   }
   virtual ~IOFWDReadRequest ();

   virtual ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb);

   // for pipeline mode
   virtual void sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size);

   virtual void initRequestParams(ReqParam & p, void * bufferMem);
   virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
   virtual void releaseBuffer(RetrievedBuffer * rb);

private:
   ReqParam param_;
   iofwdutil::HybridAllocator<4096> h;
   zoidfs::zoidfs_handle_t handle_;
};

//===========================================================================
   }
}

#endif
