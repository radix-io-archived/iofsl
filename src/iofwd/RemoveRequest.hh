#ifndef IOFWD_REMOVEREQUEST_HH
#define IOFWD_REMOVEREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
//===========================================================================

class RemoveRequest : public Request
{
public:

   typedef struct
   {
      char *              full_path;
      zoidfs::zoidfs_handle_t *   parent_handle; 
      char *              component_name;
      zoidfs::util::ZoidFSOpHint * op_hint; 
   } ReqParam; 

   RemoveRequest (int opid) : 
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
    */
   virtual void reply (const CBType & cb,
         const zoidfs::zoidfs_cache_hint_t * parent_hint) = 0;
}; 


//===========================================================================
}


#endif
