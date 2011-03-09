#ifndef IOFWDUTIL_STATS_AUTOCOUNTER_HH
#define IOFWDUTIL_STATS_AUTOCOUNTER_HH

#include "iofwdutil/stats/TimeCounter.hh"
#include "iofwdutil/stats/CounterConfigOptions.hh"

namespace iofwdutil
{
    namespace stats
    {

template<typename T, typename C=CounterConfigDefault>
class AutoCounter
{
    public:
        AutoCounter(std::string name,
                typename T::type val,
                bool timed=false) :
            counter_(T::get(name + std::string(".auto"))),
            start_(0.0),
            val_(val),
            timed_(timed),
            config_t_(counter_)
        {
            if(counter_)
            {
                /* update on counter creation */
                counter_->update(val_);

                /* start timer */
                if(timed_)
                {
                    if(timer_)
                    {
                        timer_ = TimeCounter::get(name + ".timer.auto");
                        config_timer_.reset(timer_);
                        start_ = timer_->start();
                    }
                }
            }
        }

        ~AutoCounter()
        {
            if(timer_)
            {
                /* stop timer */
                if(timed_)
                {
                    double stop = timer_->start();
                    timer_->update(stop - start_);
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
