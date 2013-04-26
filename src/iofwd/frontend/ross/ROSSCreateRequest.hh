#ifndef ROSS_ROSSFRONTEND_ROSSCREATEREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/CreateRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSCreateRequest
   : public ROSSRequest,
     public CreateRequest
{
public:
   ROSSCreateRequest(int opid)
      : ROSSRequest(), CreateRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam();

   virtual void reply(const CBType & cb, const zoidfs::zoidfs_handle_t *
         handle, int created);

   virtual ~ROSSCreateRequest();

protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t attr_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
