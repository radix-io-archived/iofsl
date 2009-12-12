#ifndef __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__
#define __SRC_SM_IOFWDLOOKUPSTATEMACHINE_HH__

#include <memory>

#include "sm/IOFWDStateMachine.hh"
#include "sm/IOFWDState.hh"
#include "sm/IOFWDStateEvents.hh"
#include "sm/IOFWDExceptionTranslator.hh"

#include "sm/SMResourceClient.hh"
#include "sm/SMManager.hh"
#include "sm/SMResourceOp.hh"
#include "sm/SMClient.hh"

#include <boost/format.hpp>
#include <unistd.h>

#include "iofwdevent/TimerResource.hh"
#include "iofwdutil/tools.hh"
#include "iofwdevent/ResourceWrapper.hh"

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

/* state machine derived from the resource client */
struct IOFWDLookupStateMachineClient : sm::SMResourceClient, IOFWDLookupStateMachine
{
    public:
        IOFWDLookupStateMachineClient(bool trace, sm::SMManager & man, iofwdevent::TimerResource & timer) : IOFWDLookupStateMachine(trace), round_(0), op_(&man), man_(man), timer_(timer)
        {
            std::cout << "created the sm test client" << std::endl;
        }

        virtual ~IOFWDLookupStateMachineClient()
        {
            std::cout << "destroyed the sm test client" << std::endl;
        }
      
        virtual bool executeClient()
        {
            ++round_;
            std::cout << "Execute... (round " << round_ << ")" << std::endl;
            if (round_ < 5)
            {
                return reschedule();
            }
            return false;
        }

        virtual void completed(bool ret)
        {
            queue_event(new IOFWDSuccessEvent());
            schedule();
            std::cout << "Client " << this << " completed call (ret=" << ret << ")" << std::endl;
        }

        bool reschedule()
        {
            op_.rearm (sm::SMResourceClientSharedPtr(dynamic_cast<sm::SMResourceClient *>(this)));
            timer_.createTimer (&op_, 3000);

            return false;
        }

protected:
   int round_;
   sm::SMResourceOp op_;
   sm::SMManager & man_;
   iofwdevent::TimerResource & timer_;
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

            *(e.cur_state_) = 0;

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
            *(e.cur_state_) = 0;

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
            *(e.cur_state_) = 1;

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
            *(e.cur_state_) = 1;

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
            *(e.cur_state_) = 1;

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
            *(e.cur_state_) = 2;

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
            *(e.cur_state_) = 2;

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
            *(e.cur_state_) = 3;

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
            *(e.cur_state_) = 3;

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
            *(e.cur_state_) = 4;

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
            *(e.cur_state_) = 4;

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
