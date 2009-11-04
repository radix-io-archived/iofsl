#ifndef IOFWD_READLINKREQUEST_HH
#define IOFWD_READLINKREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
//===========================================================================

class ReadLinkRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      uint64_t buffer_length;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam; 

   ReadLinkRequest (int opid) : 
      Request (opid)
   {
   }
   virtual ~ReadLinkRequest ()
   {
   }

   /**
    * Retrieve the request input parameters 
    */
   virtual const ReqParam & decodeParam ()  = 0; 

   /**
    * Reply with the handle or 0 if an error occurred and the handle does not
    * need to be transmitted
    */
   virtual iofwdutil::completion::CompletionID * reply (const char * buffer,
                                                        uint64_t buffer_length) = 0;
}; 


//===========================================================================
}


#endif
