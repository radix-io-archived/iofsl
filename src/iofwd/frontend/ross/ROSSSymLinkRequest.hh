#ifndef ROSS_FRONTEND_ROSSSYMLINKREQUEST_HH
#define ROSS_FRONTEND_ROSSSYMLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/SymLinkRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class ROSSSymLinkRequest
   : public ROSSRequest,
     public SymLinkRequest
{
public:
   ROSSSymLinkRequest(int opid)
      : ROSSRequest(), SymLinkRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam() ;

   virtual void reply(const CBType & cb,
                       const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                       const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~ROSSSymLinkRequest();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

//===========================================================================
   }
}

#endif
