#ifndef ROSS_ROSSFRONTEND_ROSSREMOVEREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/RemoveRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSRemoveRequest
   : public ROSSRequest,
     public RemoveRequest
{
public:
   ROSSRemoveRequest(int opid)
      : ROSSRequest(), RemoveRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam();

   virtual void reply(const CBType & cb,
           const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~ROSSRemoveRequest();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
