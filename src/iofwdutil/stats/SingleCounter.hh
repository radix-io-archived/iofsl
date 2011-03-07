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
            {
                boost::mutex::scoped_lock(mutex_);

                /* update the accumulator */
                (*acc_)(val);
            }
        }

        virtual void reset()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                delete acc_;
                acc_ = new SingleCounterAccumulator();
            }
        }

        double counter_min()
        {
            boost::mutex::scoped_lock(mutex_);
            return boost::accumulators::min(*acc_);
        }

        double counter_max()
        {
            boost::mutex::scoped_lock(mutex_);
            return boost::accumulators::max(*acc_);
        }

        double counter_mean()
        {
            boost::mutex::scoped_lock(mutex_);
            return boost::accumulators::mean(*acc_);
        }

        double counter_variance()
        {
            boost::mutex::scoped_lock(mutex_);
            return boost::accumulators::variance(*acc_);
        }

        double counter_count()
        {
            boost::mutex::scoped_lock(mutex_);
            return boost::accumulators::count(*acc_);
        }

        typedef T type;

    protected:
        /* stat accumulator */
        typedef boost::accumulators::accumulator_set< double,
                boost::accumulators::stats<boost::accumulators::tag::min,
                boost::accumulators::tag::max, boost::accumulators::tag::mean,
                boost::accumulators::tag::variance, boost::accumulators::tag::count>
                    > SingleCounterAccumulator;

        SingleCounter(std::string name, T val) :
            BaseCounter(name + std::string("_single")),
            val_(val),
            acc_(new SingleCounterAccumulator)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~SingleCounter()
        {
            delete acc_;
        }

        T val_;

        SingleCounterAccumulator * acc_;
};

    } /* stats */
} /* iofwdutil */
#endif
