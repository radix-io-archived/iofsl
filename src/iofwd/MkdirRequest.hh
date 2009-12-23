#ifndef IOFWD_MKDIRREQUEST_HH
#define IOFWD_MKDIRREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
//===========================================================================

class MkdirRequest : public Request
{
public:

   typedef struct
   {
      char * full_path;
      zoidfs::zoidfs_handle_t * parent_handle; 
      char * component_name;
      zoidfs::zoidfs_sattr_t * sattr;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam; 

   MkdirRequest (int opid) : 
      Request (opid)
   {
   }

   /**
    * Retrieve the request input parameters 
    */
   virtual const ReqParam & decodeParam ()  = 0; 

   /**
    * Reply with the handle or 0 if an error occurred and the handle does not
    * need to be transmitted
    *
    * TODO: Check this. From the signature, it seems like the new handle is
    * not returned. It would probably make sense to do so.
    */
   virtual void reply (const CBType & cb, 
         const zoidfs::zoidfs_cache_hint_t * parent_hint) = 0;
}; 


//===========================================================================
}

#endif
