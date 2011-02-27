#ifndef IOFWDCLIENT_REQUESTTRACKER_HH
#define IOFWDCLIENT_REQUESTTRACKER_HH

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/IOFWDRequest.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-async.h"

#include <boost/unordered_set.hpp>

namespace iofwdclient
{
   //========================================================================

   /**
    * Be careful in this class: think long and hard about when to use a normal
    * (IOFWDRequest *) and when to use a 
    * reference counted request pointer (IOFWDRequestPtr)...
    *
    * @TODO: either here or in the users of this class, make sure at least one
    * thread is calling zoidfs_progress () instead of waiting.
    */
   class RequestTracker
   {
      public:


         RequestTracker ();

         /**
          * Return new (unbound) request
          * The refcount is incremented before returning.
          */
         IOFWDRequest * newRequest ();

         /**
          * Decrement refcount (and free if appropriately)
          */
         void freeRequest (IOFWDRequest * ptr);


         /**
          * Return a progress CB for the specified request
          */
         IOFWDClientCB getCB (const IOFWDRequestPtr & ptr);

         /**
          * wait until status of specified request contains one of mask bits
          * Returns true if status changed or false if timeout was reached.
          * We can use a normal pointer, since this function is called from
          * within the public zoidfs_request_test function: there is no need
          * to protect from the operation completing and releasing ptr while
          * in wait since this could only happen if the user called
          * request_free on ptr, in which case it is illegal to try to wait on
          * the request.
          */
         bool wait (IOFWDRequest * ptr,
                    zoidfs::zoidfs_comp_mask_t mask,
                    zoidfs::zoidfs_timeout_t millisec);

      protected:
         /**
          * static helper function to keep callback size down
          */
         static void staticUpdateStatus (const IOFWDRequestPtr & req,
               zoidfs::zoidfs_comp_mask_t mask,
               const iofwdevent::CBException & e);

         /**
          * Called when the completion/error status of a request changes.
          */
         void updateStatus (const IOFWDRequestPtr & req,
               zoidfs::zoidfs_comp_mask_t mask,
               const iofwdevent::CBException & e);

         /**
          * Called when a request can go on the 'changed status' list;
          * Also signals any status trackers
          */
         void requestStatusChanged (const IOFWDRequestPtr & ptr);

         /**
          * Convert zoidfs timeout into a posix time type
          *  and wait appropriately
          */
         static bool smartWait (boost::condition_variable & cond,
               boost::mutex::scoped_lock & l,
               zoidfs::zoidfs_timeout_t millisec);

      protected:
         boost::mutex status_list_lock_;
         boost::condition_variable status_list_cond_;
   };

   //========================================================================
}

#endif
