#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwdutil/HybridAllocator.hh"

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
public:
   IOFWDReadRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), ReadRequest (opid)
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
