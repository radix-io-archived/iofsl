#ifndef IOFWDUTIL_WORKQUEUE_COMPLETIONTRACKER_HH
#define IOFWDUTIL_WORKQUEUE_COMPLETIONTRACKER_HH

#include <boost/thread.hpp>
#include <set>
#include <vector>

namespace iofwdutil
{
   namespace completion
   {
      class WorkQueueCompletionID;
   }

   namespace workqueue
   {
//=======================================================================
     
      class WorkItem; 
      class WorkQueueBase; 

/** 
 * Helper for the workqueues. 
 * Allows testing, tracking and waiting for ongoing operations.
 */
class CompletionTracker
{
protected:
   /// ID type; used internally
   typedef WorkItem * value_type; 
   typedef value_type ID;

   friend class WorkItem; 
   friend class WorkQueueBase;
   friend class iofwdutil::completion::WorkQueueCompletionID;

public:
   CompletionTracker (); 

   ID createID (value_type val);


   /// Test if the given ID has completed
   /// If it completed, set val to the operation value
   bool test (ID id); 

   /// Wait until the given operation has completed
   /// Return value
   void wait (ID id);


   /// Called when the operation completed
   void completed (ID id); 

   /// Return all currently completed items
   template <typename OUTPUT> 
   void testAll (OUTPUT out);

   /// Wait until all items complete
   template <typename OUTPUT>
   void waitAll (OUTPUT out); 


   /// Returns true not all items have been waited on
   /// Takes a lock on the queue
   bool isBusy () const; 

protected:
   /**
    * Return all currently completed operations. Needs to be called with lock
    * held.
    */
   template <typename OUTPUT>
   void testAll_nolock (OUTPUT out); 

   /// Needs to be called with lock held
   /// Return true if some operations completed.
   /// Does not retrieve them from the completion list
   bool someReady () const
   { return !completed_.empty (); }

protected:
   /// Number of ongoing operations
   unsigned int pending_; 

   /// List of IDs that already completed
   std::vector<ID> completed_; 

   /// List of active id;s
   std::set<ID> active_; 

   mutable boost::mutex lock_; 

   /// For signalling when completed items are available
   boost::condition_variable ready_;

}; 
   
//=======================================================================

/// Wait for all items to complete
template <typename OUTPUT>
void CompletionTracker::waitAll (OUTPUT out)
{
   boost::mutex::scoped_lock lock(lock_);
      
   if (someReady ())
         testAll_nolock (out); 

   while (pending_)
   {
      ready_.wait (lock); 
      if (someReady ())
         testAll_nolock (out); 
   }
}

/// Return the values for completed items
template <typename OUTPUT> 
void CompletionTracker::testAll_nolock (OUTPUT out)
{
   std::copy (completed_.begin(), completed_.end(), out); 
   completed_.clear (); 
}

/// Return the values for completed items
template <typename OUTPUT> 
void CompletionTracker::testAll (OUTPUT out)
{
   boost::mutex::scoped_lock  l(lock_); 
   testAll_nolock (out);
}

//=======================================================================
   }
}

#endif
