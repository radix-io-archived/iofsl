#ifndef IOFWDUTIL_WORKQUEUE_POOLWORKQUEUE_HH
#define IOFWDUTIL_WORKQUEUE_POOLWORKQUEUE_HH

#include <deque>
#include <boost/thread.hpp>
#include "WorkQueueBase.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

class PoolWorkQueue : public WorkQueueBase
{
public:
   PoolWorkQueue (unsigned int min = 0, unsigned int max = 6);

   virtual iofwdutil::completion::CompletionID * queueWork (WorkItem * item);

   virtual ~PoolWorkQueue ();

protected:

   /// Add another thread to the pool
   void createThread ();

   /// Called by the thread to destroy itself
   void shutdownThread ();

   /// Thread entry point
   void threadMain ();

protected:
   unsigned int thread_min_;
   unsigned int thread_max_;

   /// Number of threads that is working; Needs worklock_
   volatile unsigned int thread_active_;

   /// Current number of threads
   volatile unsigned int thread_current_;

   /// Shutdown flag
   volatile bool shutdown_;

   iofwdutil::IOFWDLogSource & log_;

   std::deque<WorkItem *> worklist_;

   // Lock that protects the work lists
   boost::mutex worklock_;

   /// Work condition variable
   boost::condition_variable workready_;

   boost::condition_variable shutdown_signal_;
};

//===========================================================================
   }
}
#endif
