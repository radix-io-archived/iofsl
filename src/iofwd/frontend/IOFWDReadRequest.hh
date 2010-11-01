#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwdutil/HybridAllocator.hh"

#include <boost/scoped_ptr.hpp>
#include "iofwdutil/hash/HashFactory.hh"
#include "iofwdutil/hash/SHA1Simple.hh"
#include "iofwdutil/hash/HashAutoRegister.hh"
#include "src/iofwdutil/transform/IOFWDZLib.hh"


using namespace iofwdutil;
using namespace iofwdutil::hash;

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
     bool op_hint_crc_enabled_;
     bool op_hint_compress_enabled_;

     boost::scoped_ptr<iofwdutil::transform::GenericTransform> transform_;
     boost::scoped_ptr<HashFunc> hashFunc_;

     char   **compressed_mem_;
     size_t compressed_size_;
     size_t rem_bytes_;
     size_t next_slot_;
     size_t user_callbacks_;
     size_t pipeline_ops_;
     boost::mutex mp_;

public:
   IOFWDReadRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), ReadRequest (opid),
        op_hint_crc_enabled_(false),
        op_hint_compress_enabled_(false),
        transform_(NULL),
        hashFunc_(NULL),
	compressed_mem_(NULL),
	compressed_size_(0),
	rem_bytes_(0),
        next_slot_(0),
	user_callbacks_(0),
	pipeline_ops_(0)
   {
   }
   virtual ~IOFWDReadRequest ();

   virtual ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer *
         rb);

   // for pipeline mode
   virtual void sendPipelineBufferCB(const iofwdevent::CBType cb,
         RetrievedBuffer * rb, size_t size);

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
