#ifndef IOFWD_FRONTEND_IOFWDRENAMEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDRENAMEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/RenameRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDRenameRequest
   : public IOFWDRequest,
     public RenameRequest,
     public iofwdutil::InjectPool<IOFWDRenameRequest>
{
public:
   IOFWDRenameRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), RenameRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb,
         const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
         const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~IOFWDRenameRequest ();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
