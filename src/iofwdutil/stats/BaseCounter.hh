#ifndef IOFWDUTIL_STATS_BASECOUNTER_HH
#define IOFWDUTIL_STATS_BASECOUNTER_HH

#include <string>
#include <boost/thread/mutex.hpp>

#include "iofwdutil/stats/CounterConfig.hh"

namespace iofwdutil
{
    namespace stats
    {
        class CounterTable;
        class CounterConfigOptions;

class BaseCounter
{
    public:
        virtual void print() = 0;
        virtual std::string pack() = 0;
        virtual double toDouble() = 0;
        virtual void reset() = 0;

        bool enabled()
        {
            //return enabled_;
            return true;
        }

        static std::string getID()
        {
            return std::string(".counter");
        }

    protected:
        friend class CounterTable;
        friend void configureCounter(BaseCounter * counter, bool enabled);
        friend std::string & getCounterKey(BaseCounter * counter);

        BaseCounter(std::string name, std::string config_key) :
            name_(name),
            config_key_(config_key),
            enabled_(iofwdutil::stats::CounterConfig::instance()[config_key_])
        {
        }

        virtual ~BaseCounter()
        {
        }

        /* counter name */
        std::string name_;

        /* config file key */
        std::string config_key_;

        /* counter mutex */
        boost::mutex mutex_;

        /* */
        bool enabled_;
};

void configureCounter(BaseCounter * counter,
        bool enabled);

std::string & getCounterKey(BaseCounter * counter);

    }
}

#endif
