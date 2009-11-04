#ifndef IOFWD_SETATTRREQUEST_HH
#define IOFWD_SETATTRREQUEST_HH

#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

// #include "SetAttrRequestSM.hh"

namespace iofwd
{

class SetAttrRequest : public Request
{
public:
   typedef struct
   {
     zoidfs::zoidfs_handle_t * handle;
     zoidfs::zoidfs_sattr_t * sattr;
     zoidfs::zoidfs_attr_t * attr;
     zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   SetAttrRequest (int opid) 
      : Request (opid)
   {
   }
   virtual ~SetAttrRequest ()
   {
   }

   virtual const ReqParam & decodeParam () = 0;
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_attr_t * attr) = 0;
}; 

}

#endif
