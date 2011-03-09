#ifndef SRC_IOFWDUTIL_STATS_COUNTERWINDOW_HH
#define SRC_IOFWDUTIL_STATS_COUNTERWINDOW_HH

#include "iofwdutil/stats/BaseCounter.hh"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace iofwdutil
{
    namespace stats
    {

/* stat accumulator */
typedef boost::accumulators::accumulator_set< double,
            boost::accumulators::stats<boost::accumulators::tag::min, 
            boost::accumulators::tag::max, boost::accumulators::tag::mean,
            boost::accumulators::tag::variance,
            boost::accumulators::tag::count>
            > CounterWindowAccumulator;

class CounterWindowStats
{
    public:
        CounterWindowStats(double mean=0.0,
                double var=0.0,
                double min=0.0,
                double max=0.0,
                double count=0.0) :
            mean_(mean),
            var_(var),
            min_(min),
            max_(max),
            count_(count)
        {
        }

        double mean_;
        double var_;
        double min_;
        double max_;
        double count_;

};

template<typename T, int N>
class CounterWindowCircularBuffer
{
    public:
        CounterWindowCircularBuffer() :
            size_(0),
            next_(0)
        {
            memset(buffer_, 0, sizeof(T) * N);
        }

        void add(const T & val)
        {
            if(next_ >= N)
            {
                next_ = 0;
            }

            if(size_ < N)
            {
                size_++;
            }
            buffer_[next_++] = const_cast<T &>(val);
        }

        size_t getStart()
        {
            return 0;
        }

        T get(int pos)
        {
            return buffer_[(getStart() + pos)];
        }

        size_t count()
        {
            return size_;
        }

        void clear()
        {
            size_ = 0;
            next_ = 0;
        }

        void computeStats(CounterWindowStats & stats)
        {
            CounterWindowAccumulator acc;

            size_t i = 0;

            /* add the window to the accumulator */
            for(i = getStart() ; i < count() ; i++)
            {
                acc(buffer_[i]);
            }

            /* fetch the stats from the accumulator */
            stats.mean_ = boost::accumulators::mean(acc);
            stats.var_ = boost::accumulators::variance(acc);
            stats.min_ = boost::accumulators::min(acc);
            stats.max_ = boost::accumulators::max(acc);
            stats.count_ = boost::accumulators::count(acc);
        }

        T buffer_[N];
        size_t size_;
        size_t next_;
};

template<typename T, int N>
class CounterWindow : public BaseCounter
{
    public:
        virtual void update(const T & val)
        {
            val_.add(val);

            /* update window stats */
            update_window_stats_ = true;

            /* update this accumulator */
            (*acc_)(val);
        }

        virtual double toDouble()
        {
            return 0.0;
        }

        virtual void print()
        {
            std::string line("");
            size_t count = 0;

            count = val_.count();
          
            line = boost::lexical_cast<std::string>(count); 
            for(size_t i = 0 ; i < count ; i++)
            {
                line = line + " " +
                    boost::lexical_cast<std::string>(val_.get(i));
            }

            std::cout << name_ << " " << line << std::endl;
        }

        virtual std::string pack()
        {
            std::string line("");
            size_t count = 0;

            count = val_.count();
           
            line = boost::lexical_cast<std::string>(count); 
            for(size_t i = 0 ; i < count ; i++)
            {
                line = line + " " +
                    boost::lexical_cast<std::string>(val_.get(i));
            }

            return line;
        }

        double window_min()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                if(update_window_stats_)
                {
                    val_.computeStats(stats_cache_);
                    update_window_stats_ = false;
                }
            }
            return stats_cache_.min_;
        }

        double window_max()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                if(update_window_stats_)
                {
                    val_.computeStats(stats_cache_);
                    update_window_stats_ = false;
                }
            }
            return stats_cache_.max_;
        }

        double window_mean()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                if(update_window_stats_)
                {
                    val_.computeStats(stats_cache_);
                    update_window_stats_ = false;
                }
            }
            return stats_cache_.mean_;
        }

        double window_count()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                if(update_window_stats_)
                {
                    val_.computeStats(stats_cache_);
                    update_window_stats_ = false;
                }
            }
            return stats_cache_.count_;
        }

        double window_variance()
        {
            {
                boost::mutex::scoped_lock(mutex_);
                if(update_window_stats_)
                {
                    val_.computeStats(stats_cache_);
                    update_window_stats_ = false;
                }
            }
            return stats_cache_.var_;
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

        virtual void reset()
        {
            val_.clear();
            delete acc_;
            acc_ = new CounterWindowAccumulator();
        }

    protected:
        CounterWindow(std::string name) :
            BaseCounter(name + std::string("_window_") +
                    boost::lexical_cast<std::string>(N),
                    name +
                    std::string(".window.") +
                    boost::lexical_cast<std::string>(N)),
            acc_(new CounterWindowAccumulator()),
            update_window_stats_(false)
        {
        }

        virtual ~CounterWindow()
        {
            delete acc_;
        }

        CounterWindowCircularBuffer<T, N> val_;
        CounterWindowAccumulator * acc_;

        CounterWindowStats stats_cache_;
        bool update_window_stats_;
};

    } /* stats */
} /* iofwdutil */
#endif
