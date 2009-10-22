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

#include <boost/intrusive_ptr.hpp>

#include <memory>

#include "IOFWDState.hh"
#include "IOFWDStateEvents.hh"

/*
 * IOFWDStateMachine - the base state machine for IOFWD processes
 */
template<
    class StateMachine,
    class StartState >
struct IOFWDStateMachine : boost::statechart::asynchronous_state_machine< IOFWDStateMachine< StateMachine, StartState >,
                                                                            StartState,
                                                                            boost::statechart::fifo_scheduler< >,
                                                                            std::allocator< void >,
                                                                            boost::statechart::null_exception_translator >
{
    public:
        virtual void run(); 
};

/* IOFWD State Machine interface */
template< class IOFWDStateMachineType >
struct IOFWDStateMachineManager
{
    public:
        /* create the event scheduler and processor */
        IOFWDStateMachineManager() : scheduler_( true )
        {
            ph_ = scheduler_.create_processor< IOFWDStateMachineType >();
            scheduler_.initiate_processor(ph_);
        }

        /* Terminate the scheduler and event processor */
        ~IOFWDStateMachineManager()
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
            scheduler_.queue_event(ph_, boost::intrusive_ptr< const boost::statechart::event_base >(iofwdEvent));
        }

    protected:
        boost::statechart::fifo_scheduler<> scheduler_;
        boost::statechart::fifo_scheduler<>::processor_handle ph_;
};

#endif
