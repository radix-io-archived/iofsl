#ifndef IOFWD_RESIZEREQUEST_HH
#define IOFWD_RESIZEREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{

class ResizeRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      uint64_t size;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   ResizeRequest (int opid) : Request (opid)
   {
   };

   virtual const ReqParam & decodeParam () = 0;

   virtual void reply (const CBType & cb) = 0;
};

}

#endif
