#ifndef IOFWD_LINKREQUEST_HH
#define IOFWD_LINKREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
//===========================================================================

class LinkRequest : public Request
{
public:

   typedef struct
   {
      char * from_full_path;
      zoidfs::zoidfs_handle_t * from_parent_handle;
      char * from_component_name;

      char * to_full_path;
      zoidfs::zoidfs_handle_t * to_parent_handle;
      char * to_component_name;
      zoidfs::util::ZoidFSOpHint * op_hint;
   } ReqParam;

   LinkRequest (int opid) :
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
   virtual void reply (const CBType & cb, const zoidfs::zoidfs_cache_hint_t *
         from_parent_hint, const zoidfs::zoidfs_cache_hint_t * to_parent_hint) = 0;
};

//===========================================================================
}

#endif
