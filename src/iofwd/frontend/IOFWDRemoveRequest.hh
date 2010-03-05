#ifndef IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/RemoveRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDRemoveRequest
   : public IOFWDRequest,
     public RemoveRequest,
     public iofwdutil::InjectPool<IOFWDRemoveRequest>
{
public:
   IOFWDRemoveRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), RemoveRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam ();

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~IOFWDRemoveRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
