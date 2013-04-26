#ifndef ROSS_ROSSFRONTEND_ROSSMKDIRREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSMKDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/MkdirRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSMkdirRequest
   : public ROSSRequest,
     public MkdirRequest
{
public:
   ROSSMkdirRequest(int opid)
      : ROSSRequest(), MkdirRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam();

   virtual void reply(const CBType & cb,
           const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~ROSSMkdirRequest();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
