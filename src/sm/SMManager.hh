#ifndef SM_SMMANAGER_HH
#define SM_SMMANAGER_HH

#include <vector>
#include <queue>
#include <csignal>

#include "iofwdutil/IOFWDLog.hh"
#include "SMClient.hh"
#include "iofwdevent/ThreadedResource.hh"
#include "iofwdutil/ThreadPool.hh"
#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>

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
 *
 * @TODO: remove old code from when SMManager still had a thread of its own.
 */
class SMManager
{
public:

   /**
    * Construct an SMManager with the specified number of worker threads.
    */
   SMManager (bool enable_poll = false, size_t threads  = 0);

   /**
    * Queue an item for execution.
    * Will keep the item alive until it executed.
    */
   void schedule (SMClient * client);

   /*
      Poll for item completion based on time limits
    */
   void poll(size_t minwaitms, size_t maxwaitms);

   /**
    * Immediately execute the client until it blocks.
    * If -- after execution -- the client reference count
    * == 0, it will be deleted.
    */
   void runNow (SMClient * client);

   ~SMManager ();

   void startThreads();
   void stopThreads();

   void useHighPrioTP(bool mode);

   void enableThreadPool(bool usetp);

protected:
   void poll_unprotected(size_t maxwait);

   /// Worker thread entry point.
   void workerMain ();

protected:
   bool poll_enabled_;
   bool polling_;
   size_t threads_;
   bool high_prio_tp_;

   // Lock protects the work queu
   std::queue<SMClientSharedPtr> worklist_;

   std::vector<boost::thread *> workers_;

   boost::mutex lock_;
   boost::condition_variable cond_;

   sig_atomic_t finish_;

   iofwdutil::zlog::ZLogSource & log_;

   iofwdutil::ThreadPool & tp_;

   /* polling mode locks */
   boost::mutex poll_lock_;
   boost::condition_variable poll_cond_;

   class SMWrapper
   {
        public:
            SMWrapper(SMClient * client, iofwdutil::ThreadPool & tp)
                : client_(client), tp_(tp)
            {
            }

            boost::intrusive_ptr<SMClient> client_;
            iofwdutil::ThreadPool & tp_;
            
   };

   static void submitWorkUnit(SMWrapper * w)
   {
        if(w)
        {
            if(w->client_)
            {
                /* execute the client work and drop the ref */
                do
                {
                } while (w->client_->execute());
    
                /* reschedule the thread with more work from the tp */
#ifndef USE_CRAY_TP
                boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(w->tp_));
#endif

                /* cleanup the wrapper alloc */
                delete w;

                return;
            }
        }
   }
};

//===========================================================================
}
#endif

