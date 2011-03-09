#ifndef IOFWDUTIL_STATS_BYTECOUNTER_HH
#define IOFWDUTIL_STATS_BYTECOUNTER_HH

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

class ByteCounter : public SingleCounter<uint64_t>, public CounterHelper<ByteCounter>
{
    public:
        virtual void update(const uint64_t & bytes)
        {
            if(enabled())
            {
                /* protect while we update the counter */
                {
                    //boost::mutex::scoped_lock(mutex_);

                    val_ += bytes;
                }

                SingleCounter<uint64_t>::update(bytes);
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
        friend class CounterHelper< ByteCounter>;

        ByteCounter(const std::string & name) :
            SingleCounter<uint64_t>(name + std::string("_byte"), name +
                    std::string(".byte"), 0)
        {
            std::cout << config_key_ << std::endl;
        }

        virtual ~ByteCounter()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
