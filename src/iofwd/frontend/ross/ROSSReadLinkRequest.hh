#ifndef ROSS_ROSSFRONTEND_ROSSREADLINKREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "ROSSRequest.hh"
#include "iofwd/ReadLinkRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

class ROSSReadLinkRequest
   : public ROSSRequest,
     public ReadLinkRequest
{
public:
   ROSSReadLinkRequest(int opid)
      : ROSSRequest(), ReadLinkRequest(opid)
   {
   }

   virtual const ReqParam & decodeParam() ;

   virtual void reply(const CBType & cb, const char * buffer,
                                         size_t buffer_length);

   virtual ~ROSSReadLinkRequest();

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   uint64_t buffer_length_;
   zoidfs::zoidfs_op_hint_t op_hint_;
};

//===========================================================================
   }
}

#endif
