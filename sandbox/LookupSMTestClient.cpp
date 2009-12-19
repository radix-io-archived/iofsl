#include <boost/format.hpp>
#include <unistd.h>

#include "sm/SMManager.hh"
#include "iofwdevent/TimerResource.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "sm/SMResourceOp.hh"
#include "sm/SMResourceClient.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "sm/SMClient.hh"

#include "sm/IOFWDLookupStateMachine.hh"
#include "sm/IOFWDStateEvents.hh"

using namespace sm;
using namespace iofwdevent;
using namespace iofwdutil;
using namespace boost;

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
      lsm_.queue_event(new IOFWDSuccessEvent());
      lsm_.schedule();
      std::cout << "Client " << this << " completed call (ret=" << ret << ")" << std::endl;
   }

   bool reschedule ()
   {
      op_.rearm (SMResourceClientSharedPtr(this));
      info_.timer_.createTimer (op_.callbackRef(), 3000);

      return false;
   }

   virtual bool executeClient ()
   {
      ++round_;
      std::cout << "Execute... (round " << round_ << ")" << std::endl;
      if (round_ < 5)
      {
         return reschedule ();
      }

      return false;
   }

   TestClient (ClientInfo & info, IOFWDLookupStateMachine & lsm)
        : round_(0),
        info_(info),
        op_(&info.man_),
        lsm_(lsm)
   {
      std::cout << "created the sm test client" << std::endl;
      boost::mutex::scoped_lock l(countlock_);
      ++count_;
   }

   virtual ~TestClient ()
   {
      std::cout << "destroyed the sm test client" << std::endl;
      boost::mutex::scoped_lock l (countlock_);
      if (!--count_)
         countcond_.notify_all ();
   }

   static void waitAll ()
   {
      std::cout << "wait for all the sm test clients" << std::endl;
      boost::mutex::scoped_lock l(countlock_);
      while (count_)
      {
         countcond_.wait (l);
      }
   }

protected:
   int round_;
   ClientInfo & info_;
   SMResourceOp op_;
   IOFWDLookupStateMachine & lsm_;


   static boost::mutex countlock_;
   static boost::condition_variable countcond_;
   static unsigned int count_;
};

unsigned int TestClient::count_ = 0;
boost::mutex TestClient::countlock_;
boost::condition_variable TestClient::countcond_;

IOFWDLookupStateMachine lsm(true);
IOFWDLookupStateMachine lsm2(true);

void run_sched()
{
/*    int ret = 0;
    int ret2 = 0;
    while(ret != 3i && ret2 != 3)
    {
        sleep(1);
        lsm.schedule();
        lsm2.schedule();
        ret = lsm.cur_state();
        ret2 = lsm2.cur_state();
    }
*/
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   try
   {
      ResourceWrapper timerwrap (&timer);

      SMManager man (4);

      ClientInfo info (timer, man);

      boost::thread otherThread(boost::bind( run_sched ) );

      man.schedule (new TestClient (info, lsm));

      man.startThreads ();
      sleep(7);
      man.schedule (new TestClient (info, lsm2));

      TestClient::waitAll ();

      man.stopThreads ();
      otherThread.join();

   }
   catch (...)
   {
      throw;
   }
}
