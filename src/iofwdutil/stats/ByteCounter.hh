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
            /* protect while we update the counter */
            {
                boost::mutex::scoped_lock(mutex_);

                val_ += bytes;
            }

            SingleCounter<uint64_t>::update(bytes);
        }

        virtual double toDouble()
        {
            return boost::lexical_cast<double>(val_);
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
            val_ = 0;
        }

    protected:
        friend class CounterHelper< ByteCounter>;

        ByteCounter(const std::string & name) :
            SingleCounter<uint64_t>(name + std::string("_byte"), 0)
        {
        }

        virtual ~ByteCounter()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
