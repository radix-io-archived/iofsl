/**
 * This file defines the state machine for a BMI pingpong client.
 * It is used both in the bmi-sm-pingpong tool and in the bmi unit test.
 */


#include <iostream>

#include "sm/SMClient.hh"
#include "iofwdutil/atomics.hh"
#include "iofwdutil/assert.hh"
#include "bmi-sm-pingpong-state.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/DummyResource.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "BMILink.hh"
#include "BMIInit.hh"

extern "C" {
#include <bmi.h>
}

using namespace std;

using boost::format;


namespace test
{
//===========================================================================

//===========================================================================
//===== Helper class to track number of live State machines =================
//===========================================================================

/**
 * All state machines deriving from this class will be tracked.
 * The waitAll() method will block until the last tracked client
 * exits.
 */
class TrackLive
{
public:
   TrackLive ()
   {
      ++counter_;
   }

   ~TrackLive ()
   {
      ALWAYS_ASSERT(counter_ > 0);
      if (--counter_)
         return;

      boost::mutex::scoped_lock l (lock_);
      cond_.notify_all ();
   }

   static void waitAll ()
   {
      boost::mutex::scoped_lock l (lock_);
      while (counter_)
      {
         cond_.wait (l);
      }
   }

protected:
   static boost::mutex                 lock_;
   static boost::condition_variable    cond_;

   static iofwdutil::fast_atomic<int>  counter_;
};

iofwdutil::fast_atomic<int> TrackLive::counter_ (0);
boost::mutex                TrackLive::lock_;
boost::condition_variable   TrackLive::cond_;

//===========================================================================
//=====BMI Pingpong state machine ===========================================
//===========================================================================

struct BMIPingPong : TrackLive,
                        sm::SimpleSM<BMIPingPong>
{
   typedef BMIPingPong  SELF;

   BMIPingPong (PPInput & input, bool master, BMI_addr_t mine, BMI_addr_t
         other, int tag, int iterations)
      : sm::SimpleSM<BMIPingPong>(input.manager_),
        smm_(input.manager_),
        bmi_ (input.bmi_),
        timer_ (input.timer_),
        slots_ (*this),
        master_(master),
        mine_ (mine),
        other_ (other),
        tag_ (tag),
        iter_ (0),
        log_ (iofwdutil::IOFWDLog::getSource("pingpong")),
        iterations_(iterations)
   {
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): constructor") % master_ %
            tag_ % iter_);
   }

   ~BMIPingPong ()
   {
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): destructor") % master_ %
            tag_ % iter_);

   }

   // Is called when the state machine first executes.
   void init (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): init") % master_ % tag_ % iter_);
      if (master_)
         setNextMethod (&SELF::doMasterSend);
      else
         setNextMethod (&SELF::waitSlaveReceive);
   }

   void doMasterSend (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): doMasterSend") % master_ % tag_ % iter_);

      bmi_.post_send (slots_[SENDSLOT], mine_, &iter_, sizeof (iter_),
            BMI_EXT_ALLOC, tag_*2, 0);
      bmi_.post_recv (slots_[RECEIVESLOT], other_, &buffer_, sizeof(buffer_),
            &actual_, BMI_EXT_ALLOC, tag_*2+1, 0);

      // Wait for receive to complete
      slots_.wait (RECEIVESLOT, &SELF::waitMasterReceive);
   }

   void waitMasterReceive (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): receive completed") % master_ % tag_ % iter_);
      slots_.wait (SENDSLOT, &SELF::waitMasterSend);
   }

   void waitMasterSend (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("master(%i)tag(%i)iter(%i): send completed") % master_ % tag_ % iter_);
      if (++iter_ < iterations_)
         setNextMethod (&SELF::doMasterSend);
   }

   // ============= slave ===========

   void waitSlaveReceive (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("slave(%i)tag(%i)iter(%i): receive posted") % master_ % tag_ % iter_);

      bmi_.post_recv (slots_[RECEIVESLOT], other_, &buffer_, sizeof(buffer_),
            &actual_, BMI_EXT_ALLOC, tag_*2, 0);
      slots_.wait(RECEIVESLOT, &SELF::slaveReceiveComplete);
   }

   void slaveReceiveComplete (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("slave(%i)tag(%i)iter(%i): receive completed") % master_ % tag_ % iter_);
      // send back buffer
      bmi_.post_send (slots_[SENDSLOT], mine_, &buffer_, sizeof (buffer_),
            BMI_EXT_ALLOC, tag_*2+1, 0);
      slots_.wait (SENDSLOT, &SELF::slaveComplete);
   }

   void slaveComplete (iofwdevent::CBException e)
   {
      e.check ();
      ZLOG_DEBUG(log_, format("slave(%i)tag(%i)iter(%i): send completed") % master_ % tag_ % iter_);
      if (++iter_ < iterations_)
         setNextMethod (&SELF::waitSlaveReceive);
   }

   protected:
      sm::SMManager & smm_;
      iofwdevent::BMIResource & bmi_;
      iofwdevent::TimerResource & timer_;

      enum { SENDSLOT = 0, RECEIVESLOT };

      sm::SimpleSlots<2, BMIPingPong> slots_;

      bool master_;

      // what we think the other guy is
      BMI_addr_t mine_;

      // For receiving from the other guy
      BMI_addr_t other_;
      int tag_;

      int iter_;

      iofwdevent::DummyResource dummy_;

      iofwdutil::zlog::ZLogSource & log_;
      int iterations_;

      int buffer_;
      bmi_size_t actual_;
};
//
//===========================================================================
//======= PPInput methods ===================================================
//===========================================================================

PPInput::PPInput (size_t threads, size_t UNUSED(stop_after_time),
      size_t UNUSED(stop_after_mb)) :
   manager_(threads), timerwrap_(&timer_), bmiwrap_(&bmi_)
{
}

//===========================================================================
//===========================================================================
//===========================================================================

void waitPingPongDone ()
{
   TrackLive::waitAll ();
}

sm::SMClient * createPingPongSM (PPInput & input, bool master,
      BMI_addr_t mine, BMI_addr_t remote, int tag, int iterations)
{
   return new BMIPingPong (input, master, mine, remote, tag, iterations);
}

//===========================================================================
}
