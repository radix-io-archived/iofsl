#ifndef ROSS_ROSSFRONTEND_ROSSCOMMITREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSCOMMITREQUEST_HH

#include "ROSSRequest.hh"
#include "iofwd/CommitRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

   namespace rossfrontend
   {

class ROSSCommitRequest
  : public CommitRequest,
    public ROSSRequest
{
public:
   ROSSCommitRequest(int opid)
      : CommitRequest(opid), ROSSRequest()
   {
   }

   virtual const ReqParam & decodeParam();

   virtual void reply(const CBType & cb);

   virtual ~ROSSCommitRequest();

protected:
   ReqParam param_;
   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};


   }

}

#endif
