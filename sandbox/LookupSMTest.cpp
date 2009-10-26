#include <iostream>

#include "IOFWDLookupStateMachine.hh"
#include "IOFWDStateEvents.hh"

/* small driver using libiofwdsm */
int main()
{
    /* create a manager for this state machine */
    IOFWDLookupStateMachine lsm(true);

    /* queue some events */
    lsm.queue_event(new IOFWDSuccessEvent());
    lsm.queue_event(new IOFWDSuccessEvent());
    lsm.queue_event(new IOFWDSuccessEvent());
    lsm.queue_event(new IOFWDSuccessEvent());

    /* process the events */
    lsm.schedule();

    return 0;
}
