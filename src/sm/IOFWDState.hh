#ifndef __SRC_SM_IOFWDSTATE_HH__
#define __SRC_SM_IOFWDSTATE_HH__

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/result.hpp>
#include <boost/statechart/detail/leaf_state.hpp>
#include <boost/statechart/detail/node_state.hpp>
#include <boost/statechart/detail/constructor.hpp>
#include <boost/statechart/detail/memory.hpp>
#include <boost/statechart/termination.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/state_machine.hpp>

#include "IOFWDStateEvents.hh"

/*
 * IOFWDStateMachineOps - the base operations for the IOFWD state machine
 */
struct IOFWDStateMachineOps
{
    public:
        virtual void run() const = 0;
};

/*
 * IOFWDState - the base state for the IOFWD state machine
 */
template<class MostDerivedState, class StateMachine>
struct IOFWDState : IOFWDStateMachineOps, boost::statechart::simple_state<MostDerivedState, StateMachine>
{
    public:
        IOFWDState()
        {
        }

        ~IOFWDState()
        {
        }

        void run() const
        {
            return;
        }
};

#endif
