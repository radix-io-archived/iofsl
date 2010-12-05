#ifndef IOFWD_FRONTEND_IOFWDGETATTRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/GetAttrRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDGetAttrRequest
   : public IOFWDRequest,
     public GetAttrRequest,
     public iofwdutil::InjectPool<IOFWDGetAttrRequest>
{
public:
   IOFWDGetAttrRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info, res), GetAttrRequest (opid)
   {
   }
   virtual ~IOFWDGetAttrRequest ();

   virtual const ReqParam & decodeParam () ;
   virtual void reply (const CBType & cb, const zoidfs::zoidfs_attr_t * attr);

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_attr_t attr_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

//===========================================================================
   }
}

#endif
