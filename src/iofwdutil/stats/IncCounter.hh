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
            if(enabled())
            {
                /* protect while we update the counter */
                {
                    //boost::mutex::scoped_lock(mutex_);

                    val_ += val;
                }
                SingleCounter< uint64_t >::update(val);
            }
        }

        virtual void reset()
        {
            if(enabled())
            {
                val_ = 0;
            }
        }

    protected:
        friend class CounterHelper< IncCounter >;

        IncCounter(const std::string & name) :
            SingleCounter<uint64_t>(name + std::string("_inc"), name +
                    std::string(".inc"), 0)
        {
        }

        ~IncCounter()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
