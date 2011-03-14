#ifndef ROSS_ROSSFRONTEND_ROSSSETATTRREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/SetAttrRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSSetAttrRequest
   : public ROSSRequest,
     public SetAttrRequest
{
public:
   ROSSSetAttrRequest(int opid)
      : ROSSRequest(), SetAttrRequest(opid)
   {
   }
   virtual ~ROSSSetAttrRequest();

   virtual const ReqParam & decodeParam() ;
   virtual void reply(const CBType & cb, const zoidfs::zoidfs_attr_t * attr);

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_attr_t attr_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
