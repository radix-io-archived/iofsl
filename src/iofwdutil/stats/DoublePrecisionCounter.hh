#ifndef IOFWDUTIL_STATS_DOUBLEPRECISIONCOUNTER_HH
#define IOFWDUTIL_STATS_DOUBLEPRECISIONCOUNTER_HH

#include <string>
#include "iofwdutil/stats/SingleCounter.hh"
#include "iofwdutil/stats/CounterHelper.hh"
#include "iofwdutil/tools.hh"
#include <boost/lexical_cast.hpp>
#include <iostream>

namespace iofwdutil
{
    namespace stats
    {

class DoublePrecisionCounter : public SingleCounter< double >, public
                               CounterHelper< DoublePrecisionCounter > 
{
    public:
        virtual void update(const double & val)
        {
            {
                /* protect while we update the counter */
                boost::mutex::scoped_lock(mutex_);

                val_ += val;
            }
            SingleCounter< double >::update(val);
        }

        virtual double toDouble()
        {
            return val_;
        }

        virtual std::string pack()
        {
            return boost::lexical_cast<std::string>(val_);
        }

        virtual void print()
        {
            std::cout << name_ << " " << boost::lexical_cast<std::string>(val_)
                << std::endl;
        }

        virtual void reset()
        {
            val_ = 0.0;
        }

    protected:
        friend class CounterHelper< DoublePrecisionCounter>;

        DoublePrecisionCounter(const std::string & name) :
            SingleCounter<double>(name + std::string("_double"), 0.0)
        {
        }

        virtual ~DoublePrecisionCounter()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
