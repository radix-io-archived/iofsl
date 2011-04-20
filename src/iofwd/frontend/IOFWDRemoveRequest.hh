#ifndef IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/RemoveRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDRemoveRequest
   : public IOFWDRequest,
     public RemoveRequest
{
public:
   IOFWDRemoveRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), RemoveRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam ();

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~IOFWDRemoveRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::util::ZoidFSOpHint op_hint_;
};

//===========================================================================
   }
}

#endif
