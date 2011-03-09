#ifndef IOFWDUTIL_STATS_SCOPEDTIMER_HH
#define IOFWDUTIL_STATS_SCOPEDTIMER_HH

#include "iofwdutil/stats/TimeCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

class ScopedTimer
{
    public:
        ScopedTimer(std::string name) :
            timer_(TimeCounter::get(name + std::string(".timer.scoped"))),
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
