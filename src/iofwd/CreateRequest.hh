#ifndef IOFWD_CREATEREQUEST_HH
#define IOFWD_CREATEREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
//===========================================================================

class CreateRequest : public Request
{
public:

   typedef struct
   {
      char *              full_path;
      zoidfs::zoidfs_handle_t *   parent_handle; 
      char *              component_name;
      zoidfs::zoidfs_sattr_t * attr;
   } ReqParam; 

   CreateRequest (int opid) : 
      Request (opid)
   {
   }
   virtual ~CreateRequest ()
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
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_handle_t * handle, int created) = 0; 
}; 


//===========================================================================
}


#endif
