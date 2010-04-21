#ifndef IOFWD_RENAMEREQUEST_HH
#define IOFWD_RENAMEREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
//===========================================================================

class RenameRequest : public Request
{
public:

   typedef struct
   {
      const char * from_full_path;
      zoidfs::zoidfs_handle_t * from_parent_handle;
      const char * from_component_name;

      const char * to_full_path;
      zoidfs::zoidfs_handle_t * to_parent_handle;
      const char * to_component_name;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   RenameRequest (int opid) :
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
              const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
              const zoidfs::zoidfs_cache_hint_t * to_parent_hint) = 0;
};

//===========================================================================
}

#endif
