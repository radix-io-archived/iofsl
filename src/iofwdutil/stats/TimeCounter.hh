#ifndef IOFWDUTIL_STATS_TIMECOUNTER_HH
#define IOFWDUTIL_STATS_TIMECOUNTER_HH

#include <string>
#include "iofwdutil/stats/SingleCounter.hh"
#include "iofwdutil/stats/CounterHelper.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/TimeVal.hh"
#include <boost/lexical_cast.hpp>
#include <iostream>

namespace iofwdutil
{
    namespace stats
    {

class TimeCounter : public SingleCounter< double >, public
                    CounterHelper< TimeCounter >
{
    public:
        virtual void update(const double & time)
        {
            if(enabled())
            {
                /* protect while we update the counter */
                {
                    //boost::mutex::scoped_lock(mutex_);

                    val_ += time;
                }
                SingleCounter< double >::update(time);
            }
        }

        virtual void reset()
        {
            if(enabled())
            {
                val_ = 0.0;
            }
        }

        double start()
        {
            if(enabled_)
            {
                return getCurTime();
            }
            return 0;
        }

        double stop()
        {
            if(enabled_)
            {
                return getCurTime();
            }
            return 0;
        }

    protected:
        friend class CounterHelper< TimeCounter >;

        TimeCounter(const std::string & name) :
            SingleCounter<double>(name + std::string("_time"), name +
                    std::string(".time"), 0.0)
        {
        }

        virtual ~TimeCounter()
        {
        }

        double getCurTime()
        {
            struct timespec tp;
            assert(clock_gettime(CLOCK_REALTIME, &tp) == 0);
            return TimeVal(tp.tv_sec, tp.tv_nsec).toDouble();
        }
};

    } /* stats */
} /* iofwdutil */
#endif
