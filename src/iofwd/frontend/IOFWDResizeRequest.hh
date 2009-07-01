#ifndef IOFWD_FRONTEND_IOFWDRESIZEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDRESIZEREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ResizeRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDResizeRequest
  : public ResizeRequest,
    public IOFWDRequest
{
public:
   IOFWDResizeRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : ResizeRequest(opid), IOFWDRequest (bmi, info, res)
   {
   }

   virtual const ReqParam & decodeParam ();

   virtual iofwdutil::completion::CompletionID * reply ();

   virtual ~IOFWDResizeRequest ();

protected:
   ReqParam param_;
   zoidfs::zoidfs_handle_t handle_;
   uint64_t size_;
};

//===========================================================================
   }
}

#endif
