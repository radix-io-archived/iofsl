#ifndef IOFWDEVENT_THREADEDRESOURCE_HH
#define IOFWDEVENT_THREADEDRESOURCE_HH

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <queue>
#include <csignal>
#include <memory>
#include "Resource.hh"

namespace iofwdevent
{
//===========================================================================

   /**
    * Helper class for resources that need a helper thread.
    * Takes care of thread creation, destruction.
    *
    * It also defines a condition variable and a lock; If the worker thread
    * goes to sleep, it should wait on the condition variable.
    * The lock_ and cond_ variables are meant to be used by derived classes.
    * wantShutdown () can be used to determine if the worker thread should
    * end.
    */
   class ThreadedResource : public Resource
   {
      public:

         virtual void start ();

         virtual void stop ();

         virtual bool started () const;

      protected:
         /// Can be used by the derived threadMain to determine if it should
         /// exit
         inline bool needShutdown()
         {
            boost::mutex::scoped_lock l(shutdown_lock_);
            return shutdown_;
         }

         inline bool isRunning()
         {
            boost::mutex::scoped_lock l(running_lock_);
            return running_;
         }
         
         /// This method needs to be overridden in the derived class
         virtual void threadMain () = 0;
         
         
         virtual ~ThreadedResource ();
         
         ThreadedResource ();

      private:

         void threadStart ();




      protected:
         boost::mutex lock_;
         boost::mutex shutdown_lock_;
         boost::mutex running_lock_;
         boost::condition_variable cond_;

      private:
         sig_atomic_t shutdown_;

         sig_atomic_t running_;

         std::auto_ptr<boost::thread> workerthread_;
   };

//===========================================================================
}

#endif
