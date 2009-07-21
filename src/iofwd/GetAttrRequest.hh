#ifndef IOFWD_GETATTRREQUEST_HH
#define IOFWD_GETATTRREQUEST_HH

#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

// #include "GetAttrRequestSM.hh"

namespace iofwd
{

class GetAttrRequest : public Request
{
public:
   typedef struct
   {
     zoidfs::zoidfs_handle_t * handle;
     zoidfs::zoidfs_attr_t * attr;
   } ReqParam;

   GetAttrRequest (int opid) 
      : Request (opid)
   {
   }
   virtual ~GetAttrRequest ()
   {
   }

   virtual const ReqParam & decodeParam () = 0;
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_attr_t * attr) = 0;
}; 

}

#endif
