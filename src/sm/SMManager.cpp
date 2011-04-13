#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>
#include <algorithm>

#include "iofwdutil/assert.hh"
#include "SMManager.hh"
#include "iofwdutil/tools.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>

#include <cstdio>
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


SMManager::SMManager (bool poll_enabled, size_t count) :
    poll_enabled_(poll_enabled),
    polling_(false),
    threads_(count),
    high_prio_tp_(true),
    finish_(false),
    log_(iofwdutil::IOFWDLog::getSource ("smmanager")),
    tp_(iofwdutil::ThreadPool::instance())
{
}

void SMManager::useHighPrioTP(bool mode)
{
    high_prio_tp_ = mode;
}

/* derived from the BMIResource poll() code */
void SMManager::poll(size_t minwaitms, size_t maxwaitms)
{
    // Calculate min time we want to try polling
    boost::system_time mintimeout = boost::get_system_time() +
        boost::posix_time::milliseconds(minwaitms);
    boost::system_time maxtimeout = boost::get_system_time() +
        boost::posix_time::milliseconds(maxwaitms);

    // Try to get the polling right until minwaitms has passed
    {
        boost::mutex::scoped_lock l(poll_lock_);
        while(polling_)
        {
            if(!poll_cond_.timed_wait(l, mintimeout))
            {
                // Timeout; somebody else is still polling
                return;
            }
        }
    }

    /* we're polling */
    polling_ = true;

    // We try to poll for the remainder of the time (until maxtimeout)
    boost::posix_time::time_duration polltime(maxtimeout -
            boost::get_system_time());
    size_t remaining = std::max((boost::int64_t) 0,
        polltime.total_milliseconds());

    // If we did specify a timeout and it already passed, don't try to poll.
    // Otherwise, if the timeout was zero, try to poll once.
    if(!remaining && maxwaitms != 0)
        return;

    poll_unprotected(remaining);

    // Release polling token and wake waiters
    boost::mutex::scoped_lock l(poll_lock_);

    /* we're done polling */
    polling_ = false;
    poll_cond_.notify_one();
}

void SMManager::poll_unprotected(size_t UNUSED(maxwait))
{
}

void SMManager::enableThreadPool(bool usetp)
{
    poll_enabled_ = !usetp;
}

//===========================================================================
}
