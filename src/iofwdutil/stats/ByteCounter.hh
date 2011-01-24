#ifndef SRC_IOFWDUTIL_STATS_BYTECOUNTER_HH
#define SRC_IOFWDUTIL_STATS_BYTECOUNTER_HH

#include <string>
#include "iofwdutil/stats/Counter.hh"
#include "iofwdutil/atomics.hh"
#include "iofwdutil/tools.hh"
#include <time.h>
#include <assert.h>
#include "iofwdutil/TimeVal.hh"

namespace iofwdutil
{
    namespace stats
    {

class ByteCounter
{
    public:
        ByteCounter(const std::string & name, uint64_t bytes, bool run=true) : cv_(name + std::string("_byte"), std::string("%llu")), run_(run), val_(bytes)
        {
        }

        ~ByteCounter()
        {
            cv_.get() += val_;
        }

        uint64_t curValue()
        {
            return cv_.get() + val_;
        }

        void operator+=(ByteCounter a)
        {
            val_ += a.val_;
        }

        double toDouble()
        {
            return static_cast<double>(val_);
        }

    protected:
        double getCurrent() const
        {
            struct timespec tp;
            assert(clock_gettime(CLOCK_REALTIME, &tp) == 0);
            return TimeVal(tp.tv_sec, tp.tv_nsec).toDouble();
        }

        Counter< uint64_t > cv_;
        bool run_;
        uint64_t val_;
};

    } /* stats */
} /* iofwdutil */
#endif
