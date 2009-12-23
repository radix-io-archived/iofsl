#ifndef IOFWD_FRONTEND_IOFWDSETATTRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/SetAttrRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDSetAttrRequest
   : public IOFWDRequest,
     public SetAttrRequest
{
public:
   IOFWDSetAttrRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info, res), SetAttrRequest (opid), op_hint_(NULL)
   {
   }
   virtual ~IOFWDSetAttrRequest ();

   virtual const ReqParam & decodeParam () ;
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_attr_t * attr);

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_attr_t attr_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
