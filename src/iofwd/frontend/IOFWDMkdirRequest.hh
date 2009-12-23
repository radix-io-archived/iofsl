#ifndef IOFWD_FRONTEND_IOFWDMKDIRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDMKDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/MkdirRequest.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDMkdirRequest
   : public IOFWDRequest,
     public MkdirRequest
{
public:
   IOFWDMkdirRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), MkdirRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~IOFWDMkdirRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
