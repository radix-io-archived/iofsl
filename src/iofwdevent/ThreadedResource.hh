#ifndef IOFWDEVENT_THREADEDRESOURCE_HH
#define IOFWDEVENT_THREADEDRESOURCE_HH

#include <boost/thread.hpp>
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
    */
   class ThreadedResource : public Resource
   {
      public:

         virtual void start ();

         virtual void stop ();

      protected:

         void threadStart ();

         virtual void threadMain () = 0;

         ThreadedResource ();

         virtual ~ThreadedResource ();

         bool isRunning () const 
         { return running_; }

         bool needShutdown () const
         { return shutdown_; }



         void signalStop ();

         void signalThread ();

         void waitStop ();

      private:
         sig_atomic_t shutdown_;

         sig_atomic_t running_;

         boost::mutex tlock_;
         std::auto_ptr<boost::thread> workerthread_;
   };

//===========================================================================
}

#endif
