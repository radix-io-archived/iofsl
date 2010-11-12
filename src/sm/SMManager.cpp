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

void SMManager::runNow (SMClient * client)
{
   // This is here so the client gets deleted if it runs to completion
   // in this method.
   SMClientSharedPtr ptr (client);
   do
   {
      ZLOG_DEBUG_MORE(log_,format("SMManager runNow client %p") % (void*) client);
   } while (client->execute ());
}

void SMManager::schedule (SMClient * client)
{
   ZLOG_DEBUG_MORE(log_,format("Scheduling client %p") % (void*) client);

   /* use submitWorkUnit method to avoid the tp queues */
   SMWrapper * w = new SMWrapper(client, tp_);

   /* TODO: we shouldn't need to manually manage the ref count */
   client->addref();

   if(high_prio_tp_)
    tp_.submitWorkUnit(boost::bind(&SMManager::submitWorkUnit, w), iofwdutil::ThreadPool::HIGH);
   else
    tp_.submitWorkUnit(boost::bind(&SMManager::submitWorkUnit, w), iofwdutil::ThreadPool::LOW);
}

void SMManager::startThreads()
{
    iofwdutil::ThreadPool::instance().start();
}

void SMManager::stopThreads()
{
    iofwdutil::ThreadPool::instance().reset();
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

SMManager::~SMManager ()
{
   // This shouldn't throw, but dump warning instead.
   ALWAYS_ASSERT(workers_.empty());
}


SMManager::SMManager (size_t count) 
   : threads_(count), high_prio_tp_(true),
   finish_(false),
   log_(iofwdutil::IOFWDLog::getSource ("smmanager")),
   tp_(iofwdutil::ThreadPool::instance())
{
}

void SMManager::useHighPrioTP(bool mode)
{
    high_prio_tp_ = mode;
}

//===========================================================================
}
