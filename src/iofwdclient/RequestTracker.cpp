#include "RequestTracker.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"
#include "iofwdclient/IOFWDClientCB.hh"

#include "zoidfs/zoidfs-async.h"   // for ZFS_COMP_xxxx

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <cstdio>
using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================
   //
   typedef boost::posix_time::millisec duration;


   RequestTracker::RequestTracker ()
   {
   }

   IOFWDRequest * RequestTracker::newRequest ()
   {
      return new IOFWDRequest (this);
   }

   void RequestTracker::freeRequest (IOFWDRequest * r)
   {
      r->removeref ();
   }

   bool RequestTracker::smartWait (boost::condition_variable & cond,
         boost::mutex::scoped_lock & l,
         zoidfs_timeout_t millisec)
   {
      if (millisec == ZOIDFS_TIMEOUT_NOWAIT)
      {
         cond.wait (l);
         return true;
      }
      else
      {
         return cond.timed_wait (l, duration (millisec));
      }
   }

   /**
    * Wait until our request changes to a status included in mask or has an
    * error condition.
    */
   bool RequestTracker::wait (IOFWDRequest * ptr,
         zoidfs_comp_mask_t mask,
         zoidfs_timeout_t millisec)
   {
      // We need the lock always to avoid races
      boost::mutex::scoped_lock l (status_list_lock_);

      // Wait until mask matches or until the request goes to error state
      mask = zoidfs_comp_mask_t (mask | ZFS_COMP_ERROR);
      while (! (ptr->getCompletionStatus () & mask))
      {
         if (!smartWait (status_list_cond_, l, millisec))
            return false;
      }
      return true;
   }

   void RequestTracker::updateStatus (const IOFWDRequestPtr & req,
         zoidfs_comp_mask_t mask, const iofwdevent::CBException & e)
   {
      if (e.hasException ())
      {
         // @TODO: convert exception to error code
         req->setReturnCode (ZFSERR_OTHER);
         req->setCompletionStatus (
               zoidfs_comp_mask_t (ZFS_COMP_DONE | ZFS_COMP_ERROR));
      }
      else
      {
         req->setCompletionStatus (mask);
      }
      requestStatusChanged (req);
   }

   void RequestTracker::requestStatusChanged (const IOFWDRequestPtr &
         UNUSED(req))
   {
      boost::mutex::scoped_lock l (status_list_lock_);
      status_list_cond_.notify_all ();
   }

   void RequestTracker::staticUpdateStatus (const IOFWDRequestPtr & req,
         zoidfs_comp_mask_t mask, const iofwdevent::CBException & e)
   {
      RequestTracker * t = req->getTracker ();
      ALWAYS_ASSERT(t);
      t->updateStatus (req, mask, e);
   }

   IOFWDClientCB RequestTracker::getCB (const IOFWDRequestPtr & ptr)
   {
      // Using static trampoline to keep the size of the CB bind small
      // (combined with storing a pointer to the tracker in the request)
      return boost::bind (&RequestTracker::staticUpdateStatus, ptr, _1, _2);
   }

   //========================================================================
}
