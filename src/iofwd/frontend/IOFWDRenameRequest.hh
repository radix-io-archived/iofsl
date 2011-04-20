#ifndef IOFWD_FRONTEND_IOFWDRENAMEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDRENAMEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/RenameRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDRenameRequest
   : public IOFWDRequest,
     public RenameRequest
{
public:
   IOFWDRenameRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), RenameRequest (opid)
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
   zoidfs::util::ZoidFSOpHint op_hint_;
};

//===========================================================================
   }
}

#endif
