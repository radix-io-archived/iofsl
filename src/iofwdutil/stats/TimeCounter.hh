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
            /* protect while we update the counter */
            {
                boost::mutex::scoped_lock(mutex_);

                val_ += time;
            }

            SingleCounter< double >::update(time);
        }

        virtual double toDouble()
        {
            return val_;
        }

        virtual void print()
        {
            std::cout << name_ << " " << boost::lexical_cast<std::string>(val_)
                << std::endl;
        }

        virtual std::string pack()
        {
            return boost::lexical_cast<std::string>(val_);
        }

        virtual void reset()
        {
            val_ = 0.0;
        }

        double start()
        {
            return getCurTime();
        }

        double stop()
        {
            return getCurTime();
        }

    protected:
        friend class CounterHelper< TimeCounter >;

        TimeCounter(const std::string & name) :
            SingleCounter<double>(name + std::string("_time"), 0.0)
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
