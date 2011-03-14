#ifndef ROSS_ROSSFRONTEND_ROSSRESIZEREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSRESIZEREQUEST_HH

#include "ROSSRequest.hh"
#include "iofwd/ResizeRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSResizeRequest
  : public ResizeRequest,
    public ROSSRequest
{
public:
   ROSSResizeRequest(int opid)
      : ResizeRequest(opid), ROSSRequest()
   {
   }

   virtual const ReqParam & decodeParam();

   virtual void reply(const CBType & cb);

   virtual ~ROSSResizeRequest();

protected:
   ReqParam param_;
   zoidfs::zoidfs_handle_t handle_;
   uint64_t size_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
