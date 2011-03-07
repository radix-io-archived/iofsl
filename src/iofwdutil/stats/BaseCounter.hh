#ifndef IOFWDUTIL_STATS_BASECOUNTER_HH
#define IOFWDUTIL_STATS_BASECOUNTER_HH

#include <string>
#include <boost/thread/mutex.hpp>

namespace iofwdutil
{
    namespace stats
    {
        class CounterTable;

class BaseCounter
{
    public:
        virtual void print() = 0;
        virtual std::string pack() = 0;
        virtual double toDouble() = 0;
        virtual void reset() = 0;

    protected:
        friend class CounterTable;

        BaseCounter(std::string name) :
            name_(name)
        {
        }

        virtual ~BaseCounter()
        {
        }

        std::string name_;

        boost::mutex mutex_;
};
    }
}

#endif
