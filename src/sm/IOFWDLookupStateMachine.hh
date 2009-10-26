#ifndef __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__
#define __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__

#include <memory>

#include "IOFWDStateMachine.hh"
#include "IOFWDState.hh"
#include "IOFWDStateEvents.hh"
#include "IOFWDExceptionTranslator.hh"

/* States */
struct IOFWDLookupInitState;
struct IOFWDLookupRunOpState;
struct IOFWDLookupMsgWaitState;
struct IOFWDLookupCleanupState;
struct IOFWDLookupErrorState;

/* State Machine */
struct IOFWDLookupAsyncStateMachine;

/* state machine ops */
struct IOFWDStateMachineOps;

/* this function is for testing / demo purposes only !!! */
void buggy_user_code()
{
    throw std::runtime_error("exception from user code");
}

/* state machine manager / interface for lookup operations */
struct IOFWDLookupStateMachine : IOFWDStateMachine< IOFWDLookupAsyncStateMachine >
{
    public:
        /* create the event scheduler and processor */
        IOFWDLookupStateMachine(bool trace) : IOFWDStateMachine< IOFWDLookupAsyncStateMachine >(trace)
        {
        }

        /* Terminate the scheduler and event processor */
        ~IOFWDLookupStateMachine()
        {
        }
};

struct IOFWDLookupMonitorState;

/*
 * The state machine for a Lookup request
 *
 * This is an asynchronous state machine... we queue events that are later scheduled instead of
 *  blocking on state transistions.
 *
 * We provide one method, run(), that must be implemented by each state of this state machine.
 */
struct IOFWDLookupAsyncStateMachine : boost::statechart::asynchronous_state_machine< IOFWDLookupAsyncStateMachine, IOFWDLookupInitState, boost::statechart::fifo_scheduler< >, std::allocator< void >, IOFWDExceptionTranslator< > >
{
    public:
        IOFWDLookupAsyncStateMachine(my_context ctx) : my_base(ctx)
        {
        }

        ~IOFWDLookupAsyncStateMachine()
        {
            this->terminate();
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
struct IOFWDLookupInitState : IOFWDState< IOFWDLookupInitState, IOFWDLookupAsyncStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< boost::statechart::exception_thrown >,
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
        boost::statechart::result react (const IOFWDSuccessEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupInitState -> IOFWDLookupRunOpState" << std::endl;
            }

            /* this is for testing / demo only */
            buggy_user_code();

            // advance to the RunOp state
            return transit<IOFWDLookupRunOpState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupInitState -> IOFWDLookupErrorState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
            std::cout << "IOFWDLookupInitState : exception detected" << std::endl;
            try
            {
                throw;
            }
            catch(const std::runtime_error &)
            {
                std::cout << "IOFWDLookupInitState : exception handled" << std::endl;
                std::cout << "IOFWDLookupInitState -> IOFWDLookupRunOpState" << std::endl;
                return transit<IOFWDLookupRunOpState>();
            }
            catch(...)
            {
                std::cout << "IOFWDLookupInitState : could not handle exception. fowarding to outer state..." << std::endl;
                return forward_event();
            }
        }
};

struct IOFWDLookupRunOpState : IOFWDState< IOFWDLookupRunOpState, IOFWDLookupAsyncStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< IOFWDRetryEvent >,
                boost::statechart::custom_reaction< boost::statechart::exception_thrown >,
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
        boost::statechart::result react (const IOFWDSuccessEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupRunOpState -> IOFWDLookupMsgWaitState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupMsgWaitState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupRunOpState -> IOFWDLookupRetryState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }

        boost::statechart::result react (const IOFWDRetryEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupRunOpState -> IOFWDLookupRetryState" << std::endl;
            }

            return transit<IOFWDLookupRunOpState>();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
                std::cout << "IOFWDLookupInitState : Exception Thrown" << std::endl;
                return discard_event();
        }
};

struct IOFWDLookupMsgWaitState : IOFWDState< IOFWDLookupMsgWaitState, IOFWDLookupAsyncStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< boost::statechart::exception_thrown >,
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
        boost::statechart::result react (const IOFWDSuccessEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupMsgWaitState -> IOFWDLookupCleanupState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupCleanupState>();
        }

        boost::statechart::result react (const IOFWDErrorEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupMsgWaitState -> IOFWDLookupErrorState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
                std::cout << "IOFWDLookupInitState : Exception Thrown" << std::endl;
                return discard_event();
        }
};

struct IOFWDLookupCleanupState : IOFWDState< IOFWDLookupCleanupState, IOFWDLookupAsyncStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< boost::statechart::exception_thrown >,
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
        boost::statechart::result react (const IOFWDSuccessEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupCleanupState -> DONE" << std::endl;
            }

            // terminate
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().terminate();
            return discard_event();
        }

        boost::statechart::result react (const IOFWDErrorEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupCleanupState -> IOFWDLookupErrorState" << std::endl;
            }

            // advance to the RunOp state
            return transit<IOFWDLookupErrorState>();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
                std::cout << "IOFWDLookupInitState : Exception Thrown" << std::endl;
                return discard_event();
        }
};

struct IOFWDLookupErrorState : IOFWDState< IOFWDLookupErrorState, IOFWDLookupAsyncStateMachine >
{
    public:
        /* events that will transition the machine out of this state */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< IOFWDSuccessEvent >,
                boost::statechart::custom_reaction< boost::statechart::exception_thrown >,
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
        boost::statechart::result react (const IOFWDSuccessEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupErrorState -> DONE" << std::endl;
            }

            // terminate
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().terminate();
            return discard_event();
        }

        boost::statechart::result react (const IOFWDErrorEvent & e)
        {
            // if tracing, print the transistion
            if(e.traceEnabled())
            {
                std::cout << "IOFWDLookupErrorState -> DONE" << std::endl;
            }

            // terminate
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().terminate();
            return discard_event();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
                std::cout << "IOFWDLookupInitState : Exception Thrown" << std::endl;
                return discard_event();
        }
};

#endif
