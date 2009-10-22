#ifndef __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__
#define __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__

#include "IOFWDStateMachine.hh"
#include "IOFWDState.hh"
#include "IOFWDStateEvents.hh"

/* States */
struct IOFWDLookupInitState;
struct IOFWDLookupRunOpState;
struct IOFWDLookupMsgWaitState;
struct IOFWDLookupCleanupState;
struct IOFWDLookupErrorState;

/* State Machine */
struct IOFWDLookupStateMachine;

/* state machine ops */
struct IOFWDStateMachineOps;
 
struct IOFWDLookupStateMachineManager : IOFWDStateMachineManager< IOFWDLookupStateMachine >
{
    public:
        /* create the event scheduler and processor */
        IOFWDLookupStateMachineManager()
        {
        }

        /* Terminate the scheduler and event processor */
        ~IOFWDLookupStateMachineManager()
        {
        }
};

/*
 * The state machine for a Lookup request
 *
 * This is an asynchronous state machine... we queue events that are later scheduled instead of
 *  blocking on state transistions.
 *
 * We provide one method, run(), that must be implemented by each state of this state machine.
 */
struct IOFWDLookupStateMachine : boost::statechart::asynchronous_state_machine< IOFWDLookupStateMachine, IOFWDLookupInitState >
{
    public:
        IOFWDLookupStateMachine(my_context ctx) : my_base(ctx)
        {
        }

        ~IOFWDLookupStateMachine()
        {
        }

        void run() const
        {
            /* go to the current state and execute the run() method */
            state_cast< const IOFWDStateMachineOps & >().run();
        }
};

/*
 *
 */
struct IOFWDLookupInitState : IOFWDState< IOFWDLookupInitState, IOFWDLookupStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDErrorEvent >
        > reactions;

        IOFWDLookupInitState()
        {
            // do nothing for now
        }

        ~IOFWDLookupInitState()
        {
            // do nothing for now
        }

        void run() const
        {
            // do nothing for now
            return;
        }
       
        /*
         * Event handlers
         */ 
        boost::statechart::result react (const IOFWDSuccessEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupRunOpState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }
};

struct IOFWDLookupRunOpState : IOFWDState< IOFWDLookupRunOpState, IOFWDLookupStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDRetryEvent >,
                boost::statechart::custom_reaction< IOFWDErrorEvent >
        > reactions;

        IOFWDLookupRunOpState()
        {
            // do nothing for now
        }

        ~IOFWDLookupRunOpState()
        {
            // do nothing for now
        }

        void run() const
        {
            // do nothing for now
            return;
        }

        /*
         * Event handlers
         */
        boost::statechart::result react (const IOFWDSuccessEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupMsgWaitState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }

        boost::statechart::result react (const IOFWDRetryEvent &)
        {
            return transit<IOFWDLookupRunOpState>();
        }

};

struct IOFWDLookupMsgWaitState : IOFWDState< IOFWDLookupMsgWaitState, IOFWDLookupStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDErrorEvent >
        > reactions;

        IOFWDLookupMsgWaitState()
        {
        }

        ~IOFWDLookupMsgWaitState()
        {
        }

        void run() const
        {
            return;
        }

        /*
         * Event handlers
         */
        boost::statechart::result react (const IOFWDSuccessEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupMsgWaitState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }
};

struct IOFWDLookupCleanupState : IOFWDState< IOFWDLookupCleanupState, IOFWDLookupStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDErrorEvent >
        > reactions;

        IOFWDLookupCleanupState()
        {
        }

        ~IOFWDLookupCleanupState()
        {
        }

        void run() const
        {
            return;
        }

        /*
         * Event handlers
         */
        boost::statechart::result react (const IOFWDSuccessEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupMsgWaitState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }
};

struct IOFWDLookupErrorState : IOFWDState< IOFWDLookupErrorState, IOFWDLookupStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDErrorEvent >
        > reactions;

        IOFWDLookupErrorState()
        {
        }

        ~IOFWDLookupErrorState()
        {
        }

        void run() const
        {
            return;
        }

        /*
         * Event handlers
         */
        boost::statechart::result react (const IOFWDSuccessEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupMsgWaitState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent &)
        {
            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }
};

#endif
