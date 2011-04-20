#ifndef IOFWD_FRONTEND_IOFWDSYMLINKREQUEST_HH
#define IOFWD_FRONTEND_IOFWDSYMLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/SymLinkRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDSymLinkRequest
   : public IOFWDRequest,
     public SymLinkRequest
{
public:
   IOFWDSymLinkRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), SymLinkRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb,
                       const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                       const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~IOFWDSymLinkRequest ();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::util::ZoidFSOpHint op_hint_;
};

//===========================================================================
   }
}

#endif
