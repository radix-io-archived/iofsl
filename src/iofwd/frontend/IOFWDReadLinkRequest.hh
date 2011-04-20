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
      : IOFWDRequest (info,res), ReadLinkRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   /// @TODO: Why is buffer_length 64 bit????
   virtual void reply (const CBType & cb, const char * buffer,
                                         size_t buffer_length);

   virtual ~IOFWDReadLinkRequest ();

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   uint64_t buffer_length_;
   zoidfs::util::ZoidFSOpHint op_hint_;
};

//===========================================================================
   }
}

#endif
