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
 *
 * @TODO: remove old code from when SMManager still had a thread of its own.
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
    * Immediately execute the client until it blocks.
    * If -- after execution -- the client reference count
    * == 0, it will be deleted.
    */
   void runNow (SMClient * client);

   ~SMManager ();

   void startThreads();
   void stopThreads();

   void useHighPrioTP(bool mode);
protected:


   /// Worker thread entry point.
   void workerMain ();

protected:
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

   class SMWrapper
   {
        public:
            SMWrapper(SMClient * client, iofwdutil::ThreadPool & tp)
                : client_(client), tp_(tp)
            {
            }

            SMClient * client_;
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
    
                /* TODO: we shouldn't need to manually manage the ref count */
                if (!w->client_->removeref())
                {
                    delete w->client_;
                }

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

