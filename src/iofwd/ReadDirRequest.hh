#ifndef IOFWD_READDIRREQUEST_HH
#define IOFWD_READDIRREQUEST_HH

#include "Request.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/completion/CompletionID.hh"

namespace iofwd
{
//===========================================================================

class ReadDirRequest : public Request
{
public:

   typedef struct
   {
      zoidfs::zoidfs_handle_t * handle;
      zoidfs::zoidfs_dirent_cookie_t cookie;
      uint32_t entry_count;
      zoidfs::zoidfs_dirent_t * entries;
      uint32_t flags;
      zoidfs::zoidfs_op_hint_t * op_hint;
   } ReqParam; 

   ReadDirRequest (int opid) : 
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
   virtual void reply (const CBType & cb, uint32_t entry_count,
                                zoidfs::zoidfs_dirent_t * entries,
                                zoidfs::zoidfs_cache_hint_t * cache) = 0;
}; 


//===========================================================================
}


#endif
