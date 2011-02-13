#include "iofwdclient/IOFWDClient.hh"
#include "iofwdclient/ScopedRequest.hh"

#include "iofwdutil/IOFWDLog.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   IOFWDClient::IOFWDClient ()
   {
      // we inherited log_ from out parent
      ZLOG_DEBUG (log_, "Initializing IOFWDClient");
   }

   IOFWDClient::~IOFWDClient ()
   {
      ZLOG_DEBUG (log_, "Shutting down IOFWDClient");
   }
   
   // @TODO: repeat for other functions

   int IOFWDClient::getattr (const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr, zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = igetattr (&req, handle, attr, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);
   }

   /**
    * Wait until r completes and return the operation's return code.
    * Releases r
    */
   int IOFWDClient::waitOp (zoidfs_request_t r, zoidfs_comp_mask_t mask)
   {
      // Automatically release r, no matter what
      ScopedRequest req (r);

      int ret = request_test (req, ZOIDFS_TIMEOUT_INF, mask);
      if (ret != ZFS_OK)
         return ret;

      int status;
      ret = request_get_error (req, &status);

      // get_error should never fail?
      ASSERT(ret == ZFS_OK);

      return status;
   }

   //========================================================================
}
