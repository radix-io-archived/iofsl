#ifndef SM_SMMANAGER_HH
#define SM_SMMANAGER_HH

#include <vector>
#include <boost/thread.hpp>
#include <queue>
#include <csignal>
#include "iofwdutil/IOFWDLog.hh"
#include "SMClient.hh"
#include "iofwdutil/ThreadPool.hh"

namespace sm
{
//===========================================================================

/**
 * SMManager acts as the scheduler and execution context for a collection 
 * of entities that typically need to run for a small amount of time
 * before they go back to sleep.
 *
 * The client uses a shared pointer so it will be freed automatically when 
 * it can no longer be rescheduled.
 */
class SMManager
{
public:

   /**
    * Construct an SMManager with the specified number of worker threads.
    */
   SMManager (size_t threads  = 0);


   /**
    * Queue an item for execution.
    * Will keep the item alive until it executed.
    */
   void schedule (SMClient * client);

   /**
    * Immediately execute the client
    */
   void runNow (SMClient * client);

   /// Stop all worker threads
   void stopThreads ();

   /// Start specified number of worker threads
   void startThreads (size_t threads = 0);

   ~SMManager ();

protected:


   /// Worker thread entry point.
   void workerMain ();

protected:
   size_t threads_;

   // Lock protects the work queu
   std::queue<SMClientSharedPtr> worklist_;

   std::vector<boost::thread *> workers_;

   boost::mutex lock_;
   boost::condition_variable cond_;

   sig_atomic_t finish_;

   iofwdutil::zlog::ZLogSource & log_;

   /* wrapper for the SM ops sent to the ThreadPool */
   struct SMHelper
   {
      SMHelper(SMClient * client) : client_(client)
      {
      }

      ~SMHelper()
      {
      }

      /* after running the SM, force the SM client to deallocate */ 
      void run()
      {
         client_->execute();
         client_.reset(0);
      }

      protected:
         SMClientSharedPtr client_;
   };


};

//===========================================================================
}


#endif
