#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>
#include "PoolWorkQueue.hh"
#include "WorkItem.hh"

using namespace boost; 
using namespace boost::lambda; 

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

PoolWorkQueue::PoolWorkQueue (unsigned int min, unsigned int max)
   : thread_min_(min), thread_max_(max), thread_active_(0),
   thread_current_(0), shutdown_(false),
   log_(iofwdutil::IOFWDLog::getSource ("PoolWorkQueue"))
{
}

PoolWorkQueue::~PoolWorkQueue ()
{
   // wake all threads waiting for new work
   shutdown_ = true; 

   ZLOG_DEBUG (log_, "Waiting for worker threads to shut down"); 
   // wait for all threads to shutdown
   while (thread_current_)
   {
      // Wake all worker thread sleeping on the lock
      workready_.notify_all (); 
      boost::mutex::scoped_lock l (worklock_); 
      if (!worklist_.empty())
         ZLOG_ERROR(log_,"workqueue destroyed with active tasks!"); 

      if (thread_current_)
      {
         ZLOG_DEBUG_EXTREME(log_,format("%u threads remaining...") %
            thread_current_); 
         shutdown_signal_.wait (l); 
      }
   }
   ZLOG_DEBUG (log_, "All worker threads stopped"); 
}

// Called with lock held
void PoolWorkQueue::shutdownThread ()
{
   --thread_current_; 
   // signal waiting main thread
   shutdown_signal_.notify_one(); 
}

void PoolWorkQueue::threadMain ()
{
   while (true)
   {
      boost::unique_lock<boost::mutex> lock (worklock_); 
      WorkItem  * workitem; 

      while (worklist_.empty ())
      {
         if (shutdown_)
         {
            ZLOG_DEBUG(log_, format("Shutdown requested (thread %u). Exiting.")
                  % boost::this_thread::get_id());
            shutdownThread (); 
            return; 
         }
         if (thread_current_ > thread_min_)
         {
            ZLOG_DEBUG(log_, format("No more work: destroying thread %u") 
                  % boost::this_thread::get_id());
            shutdownThread (); 
            return; 
         }
         ZLOG_DEBUG_MORE(log_, format("Thread %u: waiting for more work")
               % boost::this_thread::get_id());
         workready_.wait (lock); 
      }

      ALWAYS_ASSERT (!worklist_.empty()); 
      ALWAYS_ASSERT (thread_active_ <= thread_current_); 

      // Have lock and there is work todo
      // remove work and release lock
      workitem = worklist_.front(); 
      worklist_.pop_front (); 
      ++thread_active_; 
      lock.unlock (); 

      // Execute work
      try
      {
         ZLOG_DEBUG_EXTREME(log_,format ("Thread %i starting work on workitem %p")
                   % boost::this_thread::get_id()
                   % workitem); 
         workitem->doWork (); 
      
         completeItem (workitem); 
         
         ZLOG_DEBUG_EXTREME(log_,format ("Thread %i finished work on workitem %p")
                   % boost::this_thread::get_id()
                   % workitem); 
      }
      catch (...)
      {
         ZLOG_CRITICAL(log_, "workitem threw exception!"); 
      }

      lock.lock (); 
      --thread_active_; 
   }
   // never get here
   ALWAYS_ASSERT(false && "Should not happen!"); 
}

// called with lock held
void PoolWorkQueue::createThread ()
{

   ++thread_current_; 
   boost::thread t (boost::bind (&PoolWorkQueue::threadMain, this)); 
   ZLOG_DEBUG(log_, format("Created thread id %u") 
                  % t.get_id());
   t.detach (); 
}

void PoolWorkQueue::queueWork (WorkItem * item)
{
         ZLOG_INFO(log_,format ("Adding work to queue (item %p)") % item); 
   newItem (item); 
   
   boost::mutex::scoped_lock l (worklock_); 

   worklist_.push_back (item); 

   // Check if we need to start new threads
   if (thread_current_ < thread_max_ && thread_active_ == thread_current_)
   {
      /// The new thread will take the work; no need to signal
      createThread (); 
   }
   else
   {
      // release the lock so that 
      l.unlock (); 
      workready_.notify_one (); 
   }
}

//===========================================================================
   }
}
