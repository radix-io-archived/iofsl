#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/exception_translator.hpp>
#include <boost/statechart/result.hpp>
#include <iostream>
#include <stdexcept>

/* test states */
struct Test_InitState;
struct Test_RunOpState;
struct Test_CleanupState;
struct Test_ErrorState;

/* test state machine transition events */
struct Test_Success : boost::statechart::event< Test_Success > {};
struct Test_Error : boost::statechart::event< Test_Error > {};
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
struct TestSM : boost::statechart::state_machine<TestSM, Test_InitState>
{
    public:
        TestSM()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~TestSM()
        {
            std::cout << __FUNCTION__ << std::endl;
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
struct Test_InitState : IGetCurrentState, boost::statechart::state<Test_InitState, TestSM>
{
    public:

        /* specify the possible transitions for this state
         *
         * Success => Test_RunOpState 
         * Error   => Test_ErrorState
         */
        typedef boost::mpl::list <
                boost::statechart::transition<Test_Exception, Test_ErrorState>, 
                boost::statechart::custom_reaction<Test_Success>, 
                boost::statechart::transition<Test_Error, Test_ErrorState> 
        > reactions;

        Test_InitState(my_context ctx) : my_base( ctx )
        {
            std::cout << __FUNCTION__ << std::endl;
            try
            {
                throw std::runtime_error("test runtime exception");
            }
            catch(...)
            {
                std::cout << "handle the exception in the SM" << std::endl;
                post_event(Test_Exception());
            }
        }

        boost::statechart::result react (const Test_Success &)
        {
            std::cout << "custom event handler: Test_Success detected" << std::endl;
            return transit<Test_RunOpState>();
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

        void exit()
        {
            std::cout << "Test_InitState " << __FUNCTION__ << std::endl;
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
struct Test_RunOpState : IGetCurrentState, boost::statechart::state<Test_RunOpState, TestSM>
{
    public:
        /* specify the possible transitions for this state
         *
         * Success => Test_CleanupState 
         * Error   => Test_ErrorState
         */
        typedef boost::mpl::list <
                boost::statechart::transition<Test_Exception, Test_ErrorState>, 
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

        void exit()
        {
            std::cout << "Test_RunOpState " << __FUNCTION__ << std::endl;
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

struct Test_CleanupState : IGetCurrentState, boost::statechart::state<Test_CleanupState, TestSM>
{
    public:
        /* specify the possible transitions for this state
         *
         * Success => Terminal state of the state machine 
         * Error   => Test_ErrorState
         */
        Test_CleanupState(my_context ctx) : my_base( ctx )
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        ~Test_CleanupState()
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        void GetCurrentState() const
        {
            std::cout << "curstate == Test_CleanupState" << std::endl;
        }

        void run() const
        {
            GetCurrentState();
        }

        void exit()
        {
            std::cout << "Test_CleanupState " << __FUNCTION__ << std::endl;
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

        void exit()
        {
            std::cout << "Test_ErrorState " << __FUNCTION__ << std::endl;
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
    TestSM lsm;

    try
    {
        lsm.initiate();
        lsm.GetCurrentState();
    }
    catch (...)
    {
        std::cout << __FUNCTION__ << " " << "CAUGHT IN MAIN()" << std::endl;
    }
    lsm.process_event(Test_Success());
    lsm.GetCurrentState();
    lsm.process_event(Test_Success());
    lsm.GetCurrentState();

    return 0;
} 
