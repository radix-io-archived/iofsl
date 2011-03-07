#ifndef IOFWDUTIL_STATS_INCCOUNTER_HH
#define IOFWDUTIL_STATS_INCCOUNTER_HH

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

class IncCounter : public SingleCounter< uint64_t >, public
                   CounterHelper< IncCounter >
{
    public:
        void update(const uint64_t & val=1)
        {
            /* protect while we update the counter */
            {
                boost::mutex::scoped_lock(mutex_);

                val_ += val;
            }
            SingleCounter< uint64_t >::update(val);
        }

        double toDouble()
        {
            return boost::lexical_cast<double>(val_);
        }

        void print()
        {
            std::cout << name_ << " " << boost::lexical_cast<std::string>(val_)
                << std::endl;
        }

        std::string pack()
        {
            return boost::lexical_cast<std::string>(val_);
        }

        virtual void reset()
        {
            val_ = 0;
        }

    protected:
        friend class CounterHelper< IncCounter >;

        IncCounter(const std::string & name) :
            SingleCounter<uint64_t>(name + std::string("_inc"), 0)
        {
        }

        ~IncCounter()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
