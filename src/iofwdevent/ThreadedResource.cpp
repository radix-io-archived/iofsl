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
      this->threadMain ();
   }

   void ThreadedResource::start ()
   {
      {
        boost::mutex::scoped_lock l(running_lock_);
        ALWAYS_ASSERT(!running_);
      }

      {
        boost::mutex::scoped_lock l(shutdown_lock_);
        shutdown_ = false;
      }

      {
        boost::mutex::scoped_lock l (lock_);
        workerthread_.reset (new boost::thread (boost::bind 
               (&ThreadedResource::threadStart,this)));
      }

      {
        boost::mutex::scoped_lock l(running_lock_);
        running_ = true;
      }
   }

   void ThreadedResource::stop ()
   {
      {
        boost::mutex::scoped_lock l(running_lock_);
        ALWAYS_ASSERT(running_);
      }

      {
        boost::mutex::scoped_lock l(shutdown_lock_);
        shutdown_ = true;
      }
      cond_.notify_one ();

      {
        boost::mutex::scoped_lock l (lock_);
        workerthread_->join ();
        workerthread_.reset (0);
      }

      {
        boost::mutex::scoped_lock l(running_lock_);
        running_ = false;
      }
    }

   ThreadedResource::ThreadedResource ()
      : shutdown_(false), running_(false)
   {
   }

   ThreadedResource::~ThreadedResource ()
   {
      boost::mutex::scoped_lock l(running_lock_);
      // This will probably have to go at some point
      // since it will interfere with exception handling
      ALWAYS_ASSERT(!running_);
   }

//===========================================================================
}
