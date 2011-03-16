#ifndef IOFWDUTIL_STATS_COUNTERTABLE_HH
#define IOFWDUTIL_STATS_COUNTERTABLE_HH

#include <string>
#include <map>
#include "iofwdutil/Singleton.hh"
#include <boost/thread/mutex.hpp>
#include "iofwdutil/stats/BaseCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

class CounterTable : public iofwdutil::Singleton< CounterTable >
{
    public:
        CounterTable();

        ~CounterTable();

        void dumpAllCounters();

        void lock()
        {
            //mutex_.lock();
        }

        void unlock()
        {
            //mutex_.unlock();
        }

        /* get the counter */
        void store(std::string name, BaseCounter * c)
        {
            counters_[name] = c;
        }

        /* allocate a counter */
        BaseCounter * find(std::string name)
        {
            BaseCounter * c = NULL;
            std::map<std::string, BaseCounter * >::iterator it;

            /* get the counter */
            it = counters_.find(name);

            if(it != counters_.end())
            {
                /* hash the counter name to the counter mem */
                //c = (counters_)[name];
                c = it->second;
            }
            else
            {
                c = NULL;
            }
            return c;
        }

        void dumpCounters();
        void clearCounters();

    protected:
        boost::mutex mutex_;
        std::map< std::string, BaseCounter * > counters_;
};

    }
}
#endif
