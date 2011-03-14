#ifndef ROSS_ROSSFRONTEND_ROSSLOOKUPREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/LookupRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSLookupRequest
   : public ROSSRequest,
     public LookupRequest
{
public:
   ROSSLookupRequest (int opid)
      : ROSSRequest(), LookupRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb,
           const zoidfs::zoidfs_handle_t * handle);

   virtual ~ROSSLookupRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
