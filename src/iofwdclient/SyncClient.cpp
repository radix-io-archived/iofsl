#include "iofwdclient/SyncClient.hh"
#include "iofwdclient/ASClient.hh"

#include "iofwdclient/ScopedRequest.hh"

#include "iofwdutil/IOFWDLog.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   SyncClient::SyncClient (iofwdutil::IOFWDLogSource & log, ASClient & as)
      : log_ (log),
        asclient_ (as)
   {
      ZLOG_DEBUG (log_, "Initializing SyncClient");
   }

   SyncClient::~SyncClient ()
   {
      ZLOG_DEBUG (log_, "Shutting down SyncClient");
   }
   
   // @TODO: repeat for other functions

   int SyncClient::getattr (const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr, zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.igetattr (&req, handle, attr, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);
   }

   /**
    * Wait until r completes and return the operation's return code.
    * Releases r
    */
   int SyncClient::waitOp (zoidfs_request_t r, zoidfs_comp_mask_t mask)
   {
      // Automatically release r, no matter what
      ScopedRequest req (r);

      int ret = asclient_.request_test (req, ZOIDFS_TIMEOUT_INF, mask);
      if (ret != ZFS_OK)
         return ret;

      int status;
      ret = asclient_.request_get_error (req, &status);

      // get_error should never fail?
      ASSERT(ret == ZFS_OK);

      return status;
   }

   //========================================================================
}
