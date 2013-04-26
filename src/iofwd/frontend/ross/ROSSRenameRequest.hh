#ifndef ROSS_ROSSFRONTEND_ROSSRENAMEREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSRENAMEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/RenameRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class ROSSRenameRequest
   : public ROSSRequest,
     public RenameRequest
{
public:
   ROSSRenameRequest(int opid)
      : ROSSRequest(), RenameRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam() ;

   virtual void reply(const CBType & cb,
         const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
         const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~ROSSRenameRequest();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

//===========================================================================
   }
}

#endif
