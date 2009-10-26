#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>
#include <iostream>

struct s1;
struct s2;
struct s3;
struct s4;

struct EV1 : boost::statechart::event< EV1 > {};
struct EV2 : boost::statechart::event< EV2 > {};
struct EV3 : boost::statechart::event< EV3 > {};

struct SM : boost::statechart::state_machine<SM, s1> {};

struct s1 : boost::statechart::simple_state<s1, SM, s2>
{
    public:
        typedef boost::statechart::transition<EV1, s1> reactions;
        s1()
        {
            std::cout << "start s1!" << std::endl;
            count_ = 0;
        }
   
        int getCount() const
        {
            return count_;
        }

        int & getCount()
        {
            return count_;
        }

        void incCount()
        {
            count_++;
        }

        void decCount()
        {
            count_--;
        }

        ~s1()
        {
            std::cout << "end s1!" << std::endl;
            std::cout << "count = " << getCount() << std::endl;
        }

    private:
        int count_;
};

struct s2 : boost::statechart::simple_state<s2, s1>
{
    typedef boost::statechart::transition<EV2, s3> reactions;
    s2()
    {
        std::cout << "start s2!" << std::endl;
    }

    ~s2()
    {
        context< s1 >().getCount() += 1;
        std::cout << "end s2!" << std::endl;
    }
};

struct s3 : boost::statechart::simple_state<s3, s1>
{
    typedef boost::statechart::transition<EV3, s4> reactions;
    s3()
    {
        std::cout << "start s3!" << std::endl;
    }

    ~s3()
    {
        context< s1 >().getCount() += 1;
        std::cout << "end s3!" << std::endl;
    }
};

struct s4 : boost::statechart::simple_state<s4, s1>
{
    s4()
    {
        std::cout << "start s4!" << std::endl;
    }

    ~s4()
    {
        context< s1 >().getCount() += 1;
        std::cout << "end s4!" << std::endl;
    }
};

int main()
{
    SM mySM;

    mySM.initiate();

    mySM.process_event( EV1() );
    mySM.process_event( EV2() );
    mySM.process_event( EV3() );
    return 0;
}
