#ifndef __SRC_SM_IOFWDEXCEPTION_TRANSLATOR_HH__
#define __SRC_SM_IOFWDEXCEPTION_TRANSLATOR_HH__

#include <boost/statechart/event.hpp>
#include <boost/statechart/exception_translator.hpp>
#include <boost/statechart/result.hpp>

#include <boost/statechart/detail/rtti_policy.hpp>
#include <boost/statechart/detail/state_base.hpp>
#include <boost/statechart/detail/leaf_state.hpp>
#include <boost/statechart/detail/node_state.hpp>
#include <boost/statechart/detail/constructor.hpp>
#include <boost/statechart/detail/avoid_unused_warning.hpp>

#include "IOFWDStateEvents.hh"

#include <iostream>

template < class ExceptionEvent = boost::statechart::exception_thrown,
            class SuccessEvent = IOFWDSuccessEvent,
            class ErrorEvent = IOFWDErrorEvent >
class IOFWDExceptionTranslator
{
    public:
        IOFWDExceptionTranslator()
        {
        }

        ~IOFWDExceptionTranslator()
        {
        }

        /* exception translator core */
        template< class Action, class ExceptionEventHandler >
        boost::statechart::result operator()( Action action, ExceptionEventHandler eventHandler )
        {
            /* try to invoke the state machine action */
            try
            {
                return action();
            }
            catch(std::runtime_error)
            {
                std::cout << "caught and handled runtime_error" << std::endl;
                return eventHandler( ExceptionEvent() );
            }
            /* if an exception was detected, run the exception handling code */
            catch ( ... )
            {
                std::cout << "caught and handled unknown exception" << std::endl;
                return eventHandler( ExceptionEvent() );
            }
        }
};

#endif
