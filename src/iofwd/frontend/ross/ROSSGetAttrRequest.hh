#ifndef ROSS_ROSSFRONTEND_ROSSGETATTRREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/GetAttrRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class ROSSGetAttrRequest
   : public ROSSRequest,
     public GetAttrRequest
{
public:
   ROSSGetAttrRequest(int opid)
      : ROSSRequest(), GetAttrRequest(opid)
   {
   }
   virtual ~ROSSGetAttrRequest();

   virtual const ReqParam & decodeParam() ;
   virtual void reply(const CBType & cb, const zoidfs::zoidfs_attr_t * attr);

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
