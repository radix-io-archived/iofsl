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
   IOFWDCreateRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), CreateRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_handle_t * handle, int created);

   virtual ~IOFWDCreateRequest ();

protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t attr_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
