#include <iostream>
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

/* small driver using libiofwdsm */
int main()
{
    try
    {
        /* create the resources and the state machine manager */
        iofwdevent::TimerResource timer;
        iofwdevent::ResourceWrapper timerwrap(&timer);
        sm::SMManager man(4);

        /* create the state machine */
        IOFWDLookupStateMachineClient * lsm = new IOFWDLookupStateMachineClient(true, man, timer);

        /* schedule the state machine */
        man.schedule(lsm);

        /* start the manager */
        man.startThreads();

        /* wait for the state machine to complete */
        sleep(20);

        /* shutdown the manager threads */
        man.stopThreads();
    }
    catch(...)
    {
        throw;
    }
    return 0;
}
