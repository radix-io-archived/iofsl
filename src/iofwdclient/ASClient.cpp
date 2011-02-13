#include "iofwdclient/ASClient.hh"
#include "iofwdclient/RequestTracker.hh"
#include "iofwdclient/IOFWDRequest.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/assert.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   ASClient::ASClient ()
   {
      // We expect timeout to be at least a 32 bit type
      STATIC_ASSERT (sizeof (zoidfs_timeout_t) >= 4);
   }

   ASClient::~ASClient ()
   {
   }
   
   IOFWDRequest * ASClient::getRequest (zoidfs::zoidfs_request_t req) const
   {
      return static_cast<IOFWDRequest*> (req);
   }

   int ASClient::request_test (zoidfs_request_t request,
         zoidfs_timeout_t timeout,
         zoidfs_comp_mask_t mask)
   {
      IOFWDRequest * ptr = getRequest (request);
      if (!tracker_->wait (ptr, mask, timeout))
         return ZFSERR_TIMEOUT;
      return ZFS_OK;
   }

   /**
    * We simply decrement the reference count for the request
    * and set the request handle to ZOIDFS_REQUEST_NULL.
    *
    * Returns ZFSERR_INVAL if request is 0, otherwise ZFS_OK.
    */
   int ASClient::request_free (zoidfs_request_t * request)
   {
      ALWAYS_ASSERT(request);

      if (! (*request))
         return ZFSERR_INVAL;

      IOFWDRequest * ptr = getRequest (request);
      tracker_->freeRequest (ptr);
      *request = ZOIDFS_REQUEST_NULL;
      return ZFS_OK;
   }

   int ASClient::request_get_comp_state (zoidfs_request_t request,
         zoidfs_comp_mask_t * mask)
   {
      if (request == ZOIDFS_REQUEST_NULL || !mask)
         return ZFSERR_INVAL;

      IOFWDRequest * ptr = getRequest (request);
      *mask = ptr->getCompletionStatus ();
      return ZFS_OK;
   }

   int ASClient::igetattr (zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint)
   {
      // validate arguments
      // Can op_hint be 0?
      if (*request || !handle || !attr)
         return ZFSERR_INVAL;

      // Create request
      //   newRequest automatically increments the refcount to compensate for
      //   the lack of automatic refcounting in the C API
      IOFWDRequestPtr r (tracker_->newRequest ());

      cbgetattr (tracker_->getCB (r),
                 r->getReturnPointer (),
                 handle, attr, op_hint);

      return ZFS_OK;
   }


   //========================================================================
}
