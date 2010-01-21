#ifndef TEST_BMI_PINGPONG_STATE_HH
#define TEST_BMI_PINGPONG_STATE_HH

#include "iofwdevent/TimerResource.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "sm/SMManager.hh"
#include "sm/SMClient.hh"

namespace test
{
//===========================================================================

class PPInput
{

public:

   PPInput (size_t threads, size_t stop_after_time,
         size_t stop_after_mb);

public:
   iofwdevent::TimerResource timer_;
   iofwdevent::BMIResource bmi_;
   sm::SMManager manager_;
   
   // Stop after stop_after seconds
   size_t  stop_after_time_;
   size_t  stop_after_MB_;

protected:
   iofwdevent::ResourceWrapper timerwrap_;
   iofwdevent::ResourceWrapper bmiwrap_;

};

//===========================================================================
//===========================================================================
//===========================================================================

sm::SMClient * createPingPongSM (PPInput & input, bool master,
      BMI_addr_t p1, BMI_addr_t p2, int tag, int iterations);

void waitPingPongDone ();

//===========================================================================
}

#endif
