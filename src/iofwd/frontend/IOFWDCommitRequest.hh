#ifndef IOFWD_FRONTEND_IOFWDCOMMITREQUEST_HH
#define IOFWD_FRONTEND_IOFWDCOMMITREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/CommitRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

   namespace frontend
   {
//===========================================================================

class IOFWDCommitRequest
  : public CommitRequest,
    public IOFWDRequest,
    public iofwdutil::InjectPool<IOFWDCommitRequest>
{
public:
   IOFWDCommitRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : CommitRequest(opid), IOFWDRequest (info, res)
   {
   }

   virtual const ReqParam & decodeParam ();

   virtual void reply (const CBType & cb);

   virtual ~IOFWDCommitRequest ();

protected:
   ReqParam param_;
   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};


//===========================================================================
   }

}

#endif
