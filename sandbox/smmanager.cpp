#include <boost/format.hpp>
#include <unistd.h>

#include "sm/SMManager.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/TimerResource.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "sm/SMResourceOp.hh"
#include "sm/SMResourceClient.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "sm/SMClient.hh"

using namespace sm;
using namespace iofwdevent;
using namespace iofwdutil;
using namespace boost;

BMIResource bmi;
TimerResource timer;


struct ClientInfo
{
   ClientInfo (TimerResource & t, SMManager & m)
      : timer_(t), man_(m)
   {
   }

   TimerResource & timer_;
   SMManager & man_;
};

struct TestClient : public SMResourceClient
{
public:

   // Called with SMResourceClient locked
   virtual void completed (bool ret)
   {
      ZLOG_INFO (log_, format("Client %p completed called (ret=%i)") % this % ret);
   }

   bool reschedule ()
   {
      op_.rearm (SMResourceClientSharedPtr(this));
      info_.timer_.createTimer (op_.callbackRef(), 1000);

      return false;
   }

   virtual bool executeClient ()
   {
      ++round_;
      ZLOG_INFO (log_, format("Execute... (round %i)") % round_);

      if (round_ < 6)
         return reschedule ();

      return false;
   }

   TestClient (ClientInfo & info)
      : log_(IOFWDLog::getSource ("testclient")),
        round_(0),
        info_(info),
        op_(&info.man_)
   {
      ZLOG_INFO (log_, "Created");
      boost::mutex::scoped_lock l(countlock_);
      ++count_;
   }

   virtual ~TestClient ()
   {
      ZLOG_INFO (log_, "destroyed!");
      boost::mutex::scoped_lock l (countlock_);
      if (!--count_)
         countcond_.notify_all ();
   }

   static void waitAll ()
   {
      boost::mutex::scoped_lock l(countlock_);
      while (count_)
      {
         countcond_.wait (l);
      }
   }

protected:
   zlog::ZLogSource & log_;

   int round_;

   ClientInfo & info_;

   SMResourceOp op_;


   static boost::mutex countlock_;
   static boost::condition_variable countcond_;
   static unsigned int count_;
};

unsigned int TestClient::count_ = 0;
boost::mutex TestClient::countlock_;
boost::condition_variable TestClient::countcond_;

int main (int UNUSED(argc), char ** UNUSED(args))
{
   try
   {
      //ResourceWrapper bmiwrap (&bmi);
      ResourceWrapper timerwrap (&timer);

      SMManager man (4);

      ClientInfo info (timer, man);

      man.schedule (new TestClient (info));
      man.schedule (new TestClient (info));

      man.startThreads ();

      TestClient::waitAll ();

      man.stopThreads ();
   }
   catch (...)
   {
      throw;
   }
}
