#ifndef IOFWD_FRONTEND_IOFWDCREATEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/CreateRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDCreateRequest
   : public IOFWDRequest,
     public CreateRequest
{
public:
   IOFWDCreateRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), CreateRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual void reply (const CBType & cb, const zoidfs::zoidfs_handle_t *
         handle, int created);

   virtual ~IOFWDCreateRequest ();

protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t attr_;
   zoidfs::util::ZoidFSOpHint op_hint_;
};

//===========================================================================
   }
}

#endif
