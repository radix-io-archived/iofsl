#ifndef __SRC_SM_IOFWDSTATEEVENTS_HH__
#define __SRC_SM_IOFWDSTATEEVENTS_HH__

#include <boost/statechart/event.hpp>

/*
 * Events that cause the IOFWD state machine to transition
 */
struct IOFWDEvent : boost::statechart::event< IOFWDEvent >
{
    public:
        IOFWDEvent() : op_code(0), op_id(0), trace_(false)
        {
        }

        ~IOFWDEvent()
        {
        }

        int & Op_Code()
        {
            return op_code;
        }

        int & Op_Id()
        {
            return op_id;
        }

        void enableTrace(bool trace)
        {
            trace_ = trace;
        }

        bool traceEnabled() const
        {
            return trace_;
        }

    protected:
        int op_code; // what type of operation is this event associated with
        int op_id;   // what instance of this operation is this event associated with
        bool trace_;
};

/* Event for transition to a SUCCESS state */
struct IOFWDSuccessEvent : IOFWDEvent
{
    public:
        IOFWDSuccessEvent()
        {
        }

        ~IOFWDSuccessEvent()
        {
        }
};

/* Event for transition to an ERROR state */
struct IOFWDErrorEvent : IOFWDEvent
{
    public:
        IOFWDErrorEvent() : ret_(0), retry_(false)
        {
        }

        ~IOFWDErrorEvent()
        {
        }
        
        int getReturnCode()
        {
            return ret_;
        }

        bool retry()
        {
            return retry_;
        }

    protected:
        int ret_;       // return code, if any
        bool retry_;    // retry the event?
};

/* Event for transition to a TERMINATE state and to permanently stop the state machine */
struct IOFWDTerminateEvent : IOFWDEvent
{
    public:
        IOFWDTerminateEvent()
        {
        }
        
        ~IOFWDTerminateEvent()
        {
        }
};
 
/* Event for transition to an EXCEPTION handling state... recovery possible, maybe */
struct IOFWDExceptionEvent : IOFWDEvent
{
    public:
        IOFWDExceptionEvent()
        {
        }

        ~IOFWDExceptionEvent()
        {
        }
};

/* Event to to transition the machine to a RETRY state */
struct IOFWDRetryEvent : IOFWDEvent
{
    public:
        IOFWDRetryEvent()
        {
        }

        ~IOFWDRetryEvent()
        {
        }
};

#endif 
