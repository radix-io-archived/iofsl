#ifndef SRC_IOFWDUTIL_STATS_TIMECOUNTER_HH
#define SRC_IOFWDUTIL_STATS_TIMECOUNTER_HH

#include <string>
#include "iofwdutil/stats/Counter.hh"
#include "iofwdutil/atomics.hh"
#include "iofwdutil/tools.hh"
#include <time.h>
#include <assert.h>
#include "iofwdutil/TimeVal.hh"

namespace iofwdutil
{
    namespace stats
    {

class TimeCounter
{
    public:
        TimeCounter(const std::string & name, bool run=true) : start_(0.0), stop_(0.0), val_(0.0), cv_(name + std::string("_timer"), std::string("%f")), run_(run)
        {
            if(run_)
            {
                start();
            }
        }

        ~TimeCounter()
        {
            if(run_)
            {
                stop();
            }
            cv_.get() += (stop_ - start_);
        }

        double curValue()
        {
            return cv_.get();
        }

        void start()
        {
            start_ = getCurrent();
        }

        void stop()
        {
            stop_ = getCurrent();
        }

        void operator+=(TimeCounter a)
        {
            val_ += a.val_;
        }

        double toDouble()
        {
            return val_;
        }

    protected:
        double getCurrent() const
        {
            struct timespec tp;
            assert(clock_gettime(CLOCK_REALTIME, &tp) == 0);
            return TimeVal(tp.tv_sec, tp.tv_nsec).toDouble();
        }

        double start_;
        double stop_;
        double val_;
        Counter< double > cv_;
        bool run_;
};

    } /* stats */
} /* iofwdutil */
#endif
