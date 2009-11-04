#ifndef IOFWD_SYMLINKREQUEST_HH
#define IOFWD_SYMLINKREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
//===========================================================================

class SymLinkRequest : public Request
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

      zoidfs::zoidfs_sattr_t * sattr;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam;

   SymLinkRequest (int opid) :
      Request (opid)
   {
   }
   virtual ~SymLinkRequest ()
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
   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                                                        const zoidfs::zoidfs_cache_hint_t * to_parent_hint) = 0;
};

//===========================================================================
}

#endif
