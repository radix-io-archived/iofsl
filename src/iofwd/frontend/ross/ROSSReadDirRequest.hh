#ifndef ROSS_ROSSFRONTEND_ROSSREADDIRREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSREADDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/ReadDirRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class ROSSReadDirRequest
   : public ROSSRequest,
     public ReadDirRequest
{
public:
   ROSSReadDirRequest(int opid)
      : ROSSRequest(), ReadDirRequest(opid), entries_(NULL)
   {
   }

   virtual const ReqParam & decodeParam() ;

   virtual void reply(const CBType & cb, uint32_t entry_count,
                                   zoidfs::zoidfs_dirent_t * entries,
                                   zoidfs::zoidfs_cache_hint_t * cache);

   virtual ~ROSSReadDirRequest();

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_dirent_cookie_t cookie_;
   uint32_t entry_count_;
   zoidfs::zoidfs_dirent_t * entries_;
   uint32_t flags_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

//===========================================================================
   }
}

#endif
