#ifndef IOFWDUTIL_STATS_SCOPEDRATECOUNTER_HH
#define IOFWDUTIL_STATS_SCOPEDRATECOUNTER_HH

#include "iofwdutil/stats/TimeCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

template< typename T >
class ScopedRateCounter
{
    public:
        ScopedRateCounter(std::string name) :
            timer_(TimeCounter::get(name + "_scoped_rate")),
            start_(0.0)
        {
            start_ = timer_->start();
        }

        ~ScopedTimer()
        {
            double stop = timer_->start();
            timer_->update(stop - start_);
        }

    protected:
        TimeCounter * timer_;
        double start_;
};
    }
}
#endif
