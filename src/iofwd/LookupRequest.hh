#ifndef IOFWD_LOOKUPREQUEST_HH
#define IOFWD_LOOKUPREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
//===========================================================================

class LookupRequest : public Request
{
public:

   typedef struct
   {
      char *              full_path;
      zoidfs::zoidfs_handle_t *   parent_handle; 
      char *              component_name; 
   } ReqParam; 

   LookupRequest (int opid) : 
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
   virtual void reply (const zoidfs::zoidfs_handle_t * handle) = 0; 

   virtual ~LookupRequest (); 
}; 


//===========================================================================
}


#endif
