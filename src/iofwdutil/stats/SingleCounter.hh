#ifndef IOFWDUTIL_STATS_SINGLECOUNTER_HH
#define IOFWDUTIL_STATS_SINGLECOUNTER_HH

#include <cstdio>

#include "iofwdutil/stats/BaseCounter.hh"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>

//using namespace boost::accumulators;

namespace iofwdutil
{
    namespace stats
    {

template<typename T>
class SingleCounter : public BaseCounter
{
    public:
        virtual void update(const T & val)
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);

                /* update the accumulator */
                (*acc_)(val);
            }
        }

        virtual double toDouble()
        {
            if(enabled())
            {
                return boost::lexical_cast<double>(val_);
            }
            return 0;
        }

        virtual std::string pack()
        {
            if(enabled())
            {
                return boost::lexical_cast<std::string>(val_);
            }
            return std::string("");
        }

        virtual void print()
        {
            if(enabled())
            {
                std::cout << name_ 
                          << " "
                          << boost::lexical_cast<std::string>(val_)
                          << " " << counter_min()
                          << " " << counter_mean()
                          << " " << counter_max()
                          << std::endl;
            }
        }

        virtual void reset()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                delete acc_;
                acc_ = new SingleCounterAccumulator();
            }
        }

        double counter_min()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                return boost::accumulators::min(*acc_);
            }
            return 0;
        }

        double counter_max()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                return boost::accumulators::max(*acc_);
            }
            return 0;
        }

        double counter_mean()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                return boost::accumulators::mean(*acc_);
            }
            return 0;
        }

        double counter_variance()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                return boost::accumulators::variance(*acc_);
            }
            return 0;
        }

        double counter_count()
        {
            if(enabled())
            {
                //boost::mutex::scoped_lock(mutex_);
                return boost::accumulators::count(*acc_);
            }
            return 0;
        }

        typedef T type;

    protected:
        /* stat accumulator */
        typedef boost::accumulators::accumulator_set< double,
                boost::accumulators::stats<boost::accumulators::tag::min,
                boost::accumulators::tag::max, boost::accumulators::tag::mean,
                boost::accumulators::tag::variance, boost::accumulators::tag::count>
                    > SingleCounterAccumulator;

        SingleCounter(std::string name,
                std::string config_key,
                T val) :
            BaseCounter(name + std::string("_single"), config_key +
                    std::string(".single")),
            val_(val),
            acc_(NULL)
        {
            if(enabled())
            {
                acc_ = new SingleCounterAccumulator();
            }
        }

        ~SingleCounter()
        {
            if(enabled())
            {
                delete acc_;
            }
        }

        T val_;

        SingleCounterAccumulator * acc_;
};

    } /* stats */
} /* iofwdutil */
#endif
