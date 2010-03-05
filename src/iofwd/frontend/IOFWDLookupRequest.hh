#ifndef IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH
#define IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/LookupRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDLookupRequest
   : public IOFWDRequest,
     public LookupRequest,
     public iofwdutil::InjectPool<IOFWDLookupRequest>
{
public:
   IOFWDLookupRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), LookupRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_handle_t * handle);

   virtual ~IOFWDLookupRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
