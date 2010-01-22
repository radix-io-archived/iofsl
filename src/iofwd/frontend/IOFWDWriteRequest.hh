#ifndef IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDWRITEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwd/BMIBufferPool.hh"
#include "iofwdutil/bmi/BMIBuffer.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDWriteRequest
   : public IOFWDRequest,
     public WriteRequest
{
public:
   IOFWDWriteRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
     : IOFWDRequest (info,res), WriteRequest (opid), bmi_buffer_(NULL)
   {
   }

   virtual ~IOFWDWriteRequest ();

   virtual const ReqParam & decodeParam ();

   virtual void reply(const CBType & cb);

   // for normal mode
   virtual void recvBuffers(const CBType & cb);

   // for pipeline mode
   virtual iofwdutil::completion::CompletionID * recvPipelineBuffer(iofwdutil::bmi::BMIBuffer * buf, size_t size);

   virtual iofwdutil::bmi::BMIAddr getRequestAddr()
   {
      return addr_;
   }
private:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   iofwdutil::bmi::BMIBuffer * bmi_buffer_;
};

//===========================================================================
   }
}

#endif
