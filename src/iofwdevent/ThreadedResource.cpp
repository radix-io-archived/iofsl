#include <boost/bind.hpp>

#include "iofwdutil/assert.hh"
#include "ThreadedResource.hh"

namespace iofwdevent
{
//===========================================================================

   bool ThreadedResource::started () const
   {
      return running_;
   }

   void ThreadedResource::threadStart ()
   {
      while (!shutdown_)
      {
         this->threadMain ();
      }
   }

   void ThreadedResource::start ()
   {
      boost::mutex::scoped_lock l (tlock_);

      ALWAYS_ASSERT(!running_);
      shutdown_ = false;
      workerthread_.reset (new boost::thread (boost::bind 
               (&ThreadedResource::threadStart,this)));
      running_ = true;
   }

   void ThreadedResource::stop ()
   {
      ALWAYS_ASSERT(running_);

      signalStop ();
      waitStop ();
   }

   void ThreadedResource::signalThread ()
   {
         //workercond_.notify_all ();
   }

   void ThreadedResource::signalStop ()
   {
         boost::mutex::scoped_lock l (tlock_);

         shutdown_ = true;
         signalThread ();
   }

   void ThreadedResource::waitStop ()
   {
      signalThread ();
      workerthread_->join ();
      workerthread_.reset (0);
      running_ = false;
   }

   ThreadedResource::ThreadedResource ()
      : shutdown_(false), running_(false)
   {
   }

   ThreadedResource::~ThreadedResource ()
   {
      // This will probably have to go at some point
      // since it will interfere with exception handling
      ALWAYS_ASSERT(!running_);
   }

//===========================================================================
}
