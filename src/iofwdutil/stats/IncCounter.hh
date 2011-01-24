#ifndef SRC_IOFWDUTIL_STATS_INCCOUNTER_HH
#define SRC_IOFWDUTIL_STATS_INCCOUNTER_HH

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

class IncCounter
{
    public:
        IncCounter(const std::string & name, bool run=true) : val_(0), cv_(name + std::string("_inc"), std::string("%llu")), run_(run)
        {
            if(run)
            {
                val_ = 1;
            }
            else
            {
                val_ = 0;
                cv_.get() = 0;
            }
        }

        ~IncCounter()
        {
            (cv_.get()) += val_;
        }

        uint64_t curValue()
        {
            uint64_t v = 0;

            v = val_ + cv_.get();
        }

        void operator+=(IncCounter a)
        {
            val_ += a.val_;
        }

        double toDouble()
        {
            return static_cast<double>(val_);
        }

    protected:
        uint64_t val_;
        Counter< uint64_t  > cv_;
        bool run_;
};

    } /* stats */
} /* iofwdutil */
#endif
