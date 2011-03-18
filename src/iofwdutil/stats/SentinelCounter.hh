#ifndef IOFWDUTIL_STATS_SENTINELCOUNTER_HH
#define IOFWDUTIL_STATS_SENTINELCOUNTER_HH

#include <iostream>
#include <string>

#include "iofwdutil/stats/BaseCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

class SentinelCounter : public BaseCounter
{
    public:
        SentinelCounter(std::string name=std::string("sentinel"),
                std::string config_key=std::string("sentinel")) :
            BaseCounter(name, config_key),
            value_(0.0)
        {
        }

        virtual ~SentinelCounter()
        {
        }

        virtual void print()
        {
            std::cout << name_ << " " << value_ << std::endl;
        }

        virtual std::string pack()
        {
            return boost::lexical_cast<std::string>(value_);
        }

        virtual double toDouble()
        {
            return value_;
        }

        virtual void reset()
        {
        }

        bool enabled()
        {
            //return enabled_;
            return true;
        }

        static std::string getID()
        {
            return std::string(".sentinel");
        }

    protected:
        const double value_;
};

    }
}

#endif
