#ifndef IOFWDUTIL_STATS_SCOPEDCOUNTER_HH
#define IOFWDUTIL_STATS_SCOPEDCOUNTER_HH

#include "iofwdutil/stats/TimeCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

template<typename T>
class ScopedCounter
{
    public:
        ScopedCounter(std::string name,
                typename T::type val,
                bool timed=false) :
            counter_(T::get(name)),
            start_(0.0),
            val_(val),
            timed_(timed)
        {
            if(timed_)
            {
                timer_ = TimeCounter::get(name + "_timer");
                start_ = timer_->start();
            }
        }

        ~ScopedCounter()
        {
            counter_->update(val_);

            if(timed_)
            {
                double stop = timer_->start();
                timer_->update(stop - start_);
            }
        }

    protected:
        T * counter_;
        TimeCounter * timer_;
        double start_;
        typename T::type val_;
        bool timed_;
};
    }
}
#endif
