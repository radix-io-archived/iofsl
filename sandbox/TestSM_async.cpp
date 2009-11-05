#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/termination.hpp>
#include <boost/statechart/asynchronous_state_machine.hpp>
#include <boost/statechart/fifo_scheduler.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/exception_translator.hpp>
#include <boost/statechart/result.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <stdexcept>

/* test states */
struct Test_InitState;
struct Test_RunOpState;
struct Test_RunOpState_SubOp1a;
struct Test_RunOpState_SubOp2a;
struct Test_RunOpState_SubOp1b;
struct Test_RunOpState_SubOp2b;
struct Test_RunOpState_SubOp1c;
struct Test_RunOpState_SubOp1d;
struct Test_CleanupState;
struct Test_ErrorState;

/* test state machine transition events */
struct Test_Success : boost::statechart::event< Test_Success > {};
struct Test_RunOpState_SubOp1_Event : boost::statechart::event< Test_RunOpState_SubOp1_Event > {};
struct Test_RunOpState_SubOp2_Event : boost::statechart::event< Test_RunOpState_SubOp2_Event > {};
struct Test_Error : boost::statechart::event< Test_Error > {};
struct Test_Terminate : boost::statechart::event< Test_Terminate > {};
struct Test_Exception : boost::statechart::event< Test_Exception >
{
    public:
        Test_Exception()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~Test_Exception()
        {
            std::cout << __FUNCTION__ << std::endl;
        }      
};
struct Test_Retry : boost::statechart::event< Test_Retry > {};

struct IGetCurrentState
{
    virtual void GetCurrentState() const = 0;
    virtual void run() const = 0;
    virtual int getStateReturnCode() const = 0;
};

/* the test state machine */
struct TestSM : boost::statechart::asynchronous_state_machine<TestSM, Test_InitState>
{
    public:
        TestSM(my_context ctx) : my_base( ctx )
        {
        }

        void GetCurrentState() const
        {
            state_cast< const IGetCurrentState & >().GetCurrentState();
        }
   
        void run() const
        {
            state_cast< const IGetCurrentState & >().run();
        }

        int getStateReturnCode() const
        {
            return state_cast< const IGetCurrentState & >().getStateReturnCode(); 
        }
};

/* state #1: get request params and init process variables */
struct Test_InitState : IGetCurrentState, boost::statechart::simple_state<Test_InitState, TestSM>
{
    public:

        /* specify the possible transitions for this state
         *
         * Success => Test_RunOpState
         * Error   => Test_ErrorState
         */
        typedef boost::mpl::list <
                boost::statechart::custom_reaction< Test_Terminate >,
                boost::statechart::custom_reaction< Test_Success >,
                boost::statechart::transition< Test_Error, Test_ErrorState >
        > reactions;

        Test_InitState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        boost::statechart::result react (const Test_Success &)
        {
            return transit<Test_RunOpState>();
        }

        boost::statechart::result react (const Test_Terminate &)
        {
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().terminate();
            return discard_event();
        }

        boost::statechart::result react (const boost::statechart::exception_thrown &)
        {
            std::cout << "react Test_Exception " << __FUNCTION__ << std::endl;
            return transit<Test_ErrorState>();
        }

        boost::statechart::result react (const Test_Exception &)
        {
            std::cout << "react Test_Exception " << __FUNCTION__ << std::endl;
            return transit<Test_ErrorState>();
        }

        int getReturnCode() const
        {
            return ret_;
        }

        void GetCurrentState() const
        {
            std::cout << "curstate == Test_InitState" << std::endl;
        }

        void run() const
        {
            GetCurrentState();
        }

        ~Test_InitState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        int getStateReturnCode() const
        {
            return ret_;
        }

    protected:
        int ret_;
};

/* state #2: invoke the test */
struct Test_RunOpState : IGetCurrentState, boost::statechart::state<Test_RunOpState, TestSM,
        boost::mpl::list<Test_RunOpState_SubOp1a, Test_RunOpState_SubOp2a> >
{
    public:
        /* specify the possible transitions for this state
         *
         * Success => Test_CleanupState
         * Error   => Test_ErrorState
         */
        typedef boost::mpl::list <
                boost::statechart::termination< Test_Terminate >,
                boost::statechart::transition<Test_Success, Test_CleanupState>,
                boost::statechart::transition<Test_Error, Test_ErrorState>
        > reactions;

        Test_RunOpState(my_context ctx) : my_base( ctx )
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~Test_RunOpState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        void run() const
        {
            GetCurrentState();
        }

        void GetCurrentState() const
        {
            std::cout << "curstate == Test_RunOpState" << std::endl;
        }

        int getStateReturnCode() const
        {
            return ret_;
        }

    protected:
        int ret_;
};

struct Test_RunOpState_SubOp1a : boost::statechart::simple_state <Test_RunOpState_SubOp1a, Test_RunOpState::orthogonal< 0 > >
{
    public:
        typedef boost::mpl::list <
            boost::statechart::transition<Test_RunOpState_SubOp1_Event, Test_RunOpState_SubOp1b> > reactions;

        Test_RunOpState_SubOp1a()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_RunOpState_SubOp1b : boost::statechart::simple_state <Test_RunOpState_SubOp1b, Test_RunOpState::orthogonal< 0 > >
{
    public:
        typedef boost::mpl::list <
            boost::statechart::transition<Test_RunOpState_SubOp1_Event, Test_RunOpState_SubOp1c> > reactions;

        Test_RunOpState_SubOp1b()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_RunOpState_SubOp1c : boost::statechart::simple_state <Test_RunOpState_SubOp1c, Test_RunOpState::orthogonal< 0 > >
{
    public:
        typedef boost::mpl::list <
            boost::statechart::transition<Test_RunOpState_SubOp1_Event, Test_RunOpState_SubOp1d> > reactions;

        Test_RunOpState_SubOp1c()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_RunOpState_SubOp1d : boost::statechart::simple_state <Test_RunOpState_SubOp1d, Test_RunOpState::orthogonal< 0 > >
{
    public:
        typedef boost::mpl::list <
            boost::statechart::transition<Test_RunOpState_SubOp1_Event, Test_RunOpState_SubOp1a> > reactions;

        Test_RunOpState_SubOp1d()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_RunOpState_SubOp2a : boost::statechart::simple_state <Test_RunOpState_SubOp2a, Test_RunOpState::orthogonal< 1 > >
{
    public:
        typedef boost::mpl::list <
            boost::statechart::transition<Test_RunOpState_SubOp2_Event, Test_RunOpState_SubOp2b> > reactions;

        Test_RunOpState_SubOp2a()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_RunOpState_SubOp2b : boost::statechart::simple_state <Test_RunOpState_SubOp2b, Test_RunOpState::orthogonal< 1 > >
{
    public:
        Test_RunOpState_SubOp2b()
        {
            std::cout << __FUNCTION__ << std::endl;
        }
};

struct Test_CleanupState : IGetCurrentState, boost::statechart::state<Test_CleanupState, TestSM>
{
    public:
        /* specify the possible transitions for this state
         *
         * Success => Terminal state of the state machine
         * Error   => Test_ErrorState
         */

        typedef boost::mpl::list <
                boost::statechart::custom_reaction< Test_Terminate >,
                boost::statechart::custom_reaction< Test_Success >,
                boost::statechart::transition< Test_Error, Test_ErrorState >
        > reactions;

        Test_CleanupState(my_context ctx) : my_base( ctx )
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~Test_CleanupState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        boost::statechart::result react(const Test_Success &)
        {
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().queue_event(machine.my_handle(), (  boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Terminate() )));
            return discard_event();
        }

        boost::statechart::result react(const Test_Terminate &)
        {
            outermost_context_type & machine = outermost_context();
            machine.my_scheduler().terminate();
            return discard_event();
        }
        void GetCurrentState() const
        {
            std::cout << "curstate == Test_CleanupState" << std::endl;
        }

        void run() const
        {
            GetCurrentState();
        }

        int getStateReturnCode() const
        {
            return ret_;
        }

    protected:
        int ret_;
};

struct Test_ErrorState : IGetCurrentState, boost::statechart::state<Test_ErrorState, TestSM>
{
    public:
        typedef boost::mpl::list <
                boost::statechart::termination< Test_Terminate >
        > reactions;

        /* Terminal state of the state machine */
        Test_ErrorState(my_context ctx) : my_base( ctx )
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~Test_ErrorState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        void GetCurrentState() const
        {
            std::cout << "curstate == Test_ErrorState" << std::endl;
        }

        void run() const
        {
            GetCurrentState();
        }

        int getStateReturnCode() const
        {
            return ret_;
        }

    protected:
        int ret_;
};

int main()
{
    boost::statechart::fifo_scheduler<> sched1( true );
    boost::statechart::fifo_scheduler<> sched2( true );

    boost::statechart::fifo_scheduler<>::processor_handle p1 = sched1.create_processor< TestSM >(/* ... */);
    sched1.initiate_processor( p1 );
    boost::statechart::fifo_scheduler<>::processor_handle p2 = sched2.create_processor< TestSM >();
    sched2.initiate_processor( p2 );

    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp2_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp2_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp2_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp1_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp2_Event() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));

    sched2.queue_event( p2, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched2.queue_event( p2, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched2.queue_event( p2, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));
    sched1.queue_event( p1, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_RunOpState_SubOp2_Event() ));
    sched2.queue_event( p2, boost::intrusive_ptr< const boost::statechart::event_base >(new Test_Success() ));

    boost::thread ot1( boost::bind(&boost::statechart::fifo_scheduler<>::operator(), &sched1, 0) );
    boost::thread ot2( boost::bind(&boost::statechart::fifo_scheduler<>::operator(), &sched2, 0) );
    //sched1();
    //sched2();
    ot1.join();
    ot2.join();

    return 0;
}
