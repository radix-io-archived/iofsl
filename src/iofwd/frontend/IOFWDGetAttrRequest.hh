#ifndef IOFWD_FRONTEND_IOFWDGETATTRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/GetAttrRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDGetAttrRequest
   : public IOFWDRequest,
     public GetAttrRequest
{
public:
   IOFWDGetAttrRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info, res), GetAttrRequest (opid)
   {
   }
   virtual ~IOFWDGetAttrRequest ();

   virtual const ReqParam & decodeParam () ;
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_attr_t * attr);

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_attr_t attr_;
};

//===========================================================================
   }
}

#endif
