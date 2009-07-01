#ifndef IOFWD_RESIZEREQUEST_HH
#define IOFWD_RESIZEREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{

class ResizeRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      uint64_t size;
   } ReqParam;

   ResizeRequest (int opid) : Request (opid)
   {
   };
   virtual ~ResizeRequest ()
   {
   }

   virtual const ReqParam & decodeParam () = 0;

   virtual iofwdutil::completion::CompletionID * reply () = 0; 
};

}

#endif
