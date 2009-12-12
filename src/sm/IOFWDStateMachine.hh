#ifndef __SRC_SM_IOFWDSTATEMACHINE_HH__
#define __SRC_SM_IOFWDSTATEMACHINE_HH__

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/fifo_scheduler.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/result.hpp>
#include <boost/statechart/null_exception_translator.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <boost/intrusive_ptr.hpp>

#include <memory>

#include "sm/IOFWDState.hh"
#include "sm/IOFWDStateEvents.hh"

#include "sm/SMResourceClient.hh"
#include "sm/SMManager.hh"
#include "sm/SMResourceOp.hh"
#include "sm/SMClient.hh"

/* IOFWD State Machine interface */
template< class IOFWDStateMachineType >
struct IOFWDStateMachine
{
    public:
        /* create the event scheduler and processor */
        /* note: scheduler will not block if the state machine is in a non terminal state */
        IOFWDStateMachine(bool trace) : scheduler_( false ), trace_(trace), cur_state_(0)
        {
            ph_ = scheduler_.create_processor< IOFWDStateMachineType >();
            scheduler_.initiate_processor(ph_);
        }

        /* Terminate the scheduler and event processor */
        ~IOFWDStateMachine()
        {
            scheduler_.destroy_processor(ph_);
            scheduler_.terminate();
        }

        /* initiate event scheduling and state transitions */
        int schedule()
        {
            return scheduler_();
        }

        void queue_event(IOFWDEvent * iofwdEvent)
        {
            iofwdEvent->enableTrace(trace_);
            iofwdEvent->setCurStateVar(&cur_state_);
            scheduler_.queue_event(ph_, boost::intrusive_ptr< const boost::statechart::event_base >(iofwdEvent));
        }

        int cur_state() const
        {
            return cur_state_;
        }
    protected:
        boost::statechart::fifo_scheduler<> scheduler_;
        boost::statechart::fifo_scheduler<>::processor_handle ph_;
   
        bool trace_;
        int cur_state_;
};

/*
 * IOFWDStateMachine - the base state machine for IOFWD processes
 */
template<
    class StateMachine,
    class StartState,
    class StateScheduler = boost::statechart::fifo_scheduler< >,
    class StateAllocator = std::allocator< void >,
    class StateExceptionTrans = boost::statechart::exception_translator<>
     >
struct IOFWDAsyncStateMachine : boost::statechart::asynchronous_state_machine< IOFWDAsyncStateMachine< StateMachine, StartState, StateScheduler, StateAllocator, StateExceptionTrans>,
                                                                            StartState,
                                                                            StateScheduler,
                                                                            StateAllocator,
                                                                            StateExceptionTrans >
{
    public:
        IOFWDAsyncStateMachine()
        {
        }

        ~IOFWDAsyncStateMachine()
        {
        }
};

#endif
