#ifndef IOFWD_COMMITREQUEST_HH
#define IOFWD_COMMITREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{

class CommitRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
   } ReqParam;

   CommitRequest (int opid) : Request (opid)
   {
   };

   virtual const ReqParam & decodeParam () = 0;

   virtual iofwdutil::completion::CompletionID * reply () = 0; 

   virtual ~CommitRequest (); 
};

}

#endif
