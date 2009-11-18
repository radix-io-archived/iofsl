#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>
#include <algorithm>

#include "iofwdutil/assert.hh"
#include "SMManager.hh"

using boost::format;

namespace sm
{
//===========================================================================

void SMManager::schedule (SMClient * client)
{
   ZLOG_DEBUG_MORE(log_,format("Scheduling client %p") % (void*) client);

   boost::mutex::scoped_lock l (lock_);

   worklist_.push (SMClientSharedPtr (client));
   // Notify a worker thread.
   cond_.notify_one ();
}

void SMManager::workerMain ()
{
   boost::mutex::scoped_lock l(lock_);
   
   ZLOG_DEBUG_EXTREME(log_, "Worker thread started");

   while (!finish_)
   {
      while (!finish_ && worklist_.empty ())
      {
         ZLOG_DEBUG_EXTREME(log_, "Worker thread going to sleep");
         cond_.wait (l);
      }
      if (finish_)
         return;

      ALWAYS_ASSERT(!worklist_.empty ());

      // Store work in shared pointer so we keep it alive
      // when removing it from the worklist
      SMClientSharedPtr ptr (worklist_.front());
      worklist_.pop ();


      // drop lock before executing payload
      l.unlock ();

      try
      {
         do
         {
            ZLOG_DEBUG_MORE(log_, format("Executing client %p") % ptr.get());
         // We execute the SMClient as long as it returns true
         } while (ptr->execute ());

         // remove reference
         // If the SMClient did not increase its reference count
         // (for example by registering with a resource)
         // it will be destroyed here
         ptr.reset(0);
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "SMClient should not throw!");
      }

      l.lock ();
   }
   
   ZLOG_DEBUG_EXTREME(log_, "Worker thread exiting");
}

void SMManager::startThreads (size_t count)
{
   ALWAYS_ASSERT(workers_.empty());

   if (!count)
      count = threads_;

   ZLOG_DEBUG(log_, format("Starting %i threads...") % count);
   workers_.reserve (count);
   for (size_t i=0; i<count; ++i)
   {
      workers_.push_back (
            new boost::thread (boost::bind (&SMManager::workerMain, this)));
   }
}

void SMManager::stopThreads ()
{
   ZLOG_DEBUG(log_, "Stopping threads...");
   finish_ = true;

   {
      boost::mutex::scoped_lock l(lock_);
      cond_.notify_all ();
   }
   for_each (workers_.begin(), workers_.end(),
         boost::bind(&boost::thread::join, _1));
   for_each (workers_.begin(), workers_.end(),
         boost::bind(&operator delete, _1));
   workers_.clear();
}

SMManager::~SMManager ()
{
   // This shouldn't throw, but dump warning instead.
   ALWAYS_ASSERT(workers_.empty());
}


SMManager::SMManager (size_t count) 
   : threads_(count),
   finish_(false),
   log_(iofwdutil::IOFWDLog::getSource ("smmanager"))
{
}

//===========================================================================
}
