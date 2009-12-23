#ifndef IOFWD_FRONTEND_IOFWDLINKREQUEST_HH
#define IOFWD_FRONTEND_IOFWDLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/LinkRequest.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDLinkRequest
   : public IOFWDRequest,
     public LinkRequest
{
public:
   IOFWDLinkRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : IOFWDRequest (info,res), LinkRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                                                        const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~IOFWDLinkRequest ();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_sattr_t sattr_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
};

//===========================================================================
   }
}

#endif
