#ifndef IOFWD_FRONTEND_IOFWDREADLINKREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/ReadLinkRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDReadLinkRequest
   : public IOFWDRequest,
     public ReadLinkRequest
{
public:
   IOFWDReadLinkRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), ReadLinkRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (const char * buffer,
                                                        uint64_t buffer_length);

   virtual ~IOFWDReadLinkRequest ();

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   uint64_t buffer_length_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
