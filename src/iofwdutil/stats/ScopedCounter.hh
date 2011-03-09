#ifndef IOFWDUTIL_STATS_SCOPEDCOUNTER_HH
#define IOFWDUTIL_STATS_SCOPEDCOUNTER_HH

#include "iofwdutil/stats/TimeCounter.hh"
#include "iofwdutil/stats/CounterConfigOptions.hh"

namespace iofwdutil
{
    namespace stats
    {

template<typename T, typename C=CounterConfigDefault>
class ScopedCounter
{
    public:
        ScopedCounter(std::string name,
                typename T::type val,
                bool timed=false) :
            counter_(T::get(name + std::string(".scoped"))),
            start_(0.0),
            val_(val),
            timed_(timed),
            config_t_(counter_)
        {
            if(timed_)
            {
                timer_ = TimeCounter::get(name + ".timer.scoped");
                config_timer_.reset(timer_);
                if(timer_)
                {
                    start_ = timer_->start();
                }
            }
        }

        ~ScopedCounter()
        {
            if(counter_)
            {
                counter_->update(val_);

                if(timed_)
                {
                    if(timer_)
                    {
                        double stop = timer_->start();
                        timer_->update(stop - start_);
                    }
                }
            }
        }

    protected:
        T * counter_;
        TimeCounter * timer_;
        double start_;
        typename T::type val_;
        bool timed_;

        /* counter config */
        C config_t_;
        C config_timer_;
};
    }
}
#endif
