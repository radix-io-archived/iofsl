#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwd/BMIBufferPool.hh"
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
      : IOFWDRequest (info,res), ReadRequest (opid), bmi_buffer_(NULL)
   {
   }
   virtual ~IOFWDReadRequest ();

   virtual const ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void sendBuffers(const CBType & cb);

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * sendPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size);
   virtual void sendPipelineBufferCB(iofwdevent::CBType cb, iofwdutil::bmi::BMIBuffer * buf, size_t size);

   virtual iofwdutil::bmi::BMIAddr getRequestAddr()
   {
      return addr_;
   }

private:
   ReqParam param_;
   iofwdutil::HybridAllocator<4096> h;
   zoidfs::zoidfs_handle_t handle_;
   iofwdutil::bmi::BMIBuffer * bmi_buffer_;
};

//===========================================================================
   }
}

#endif
