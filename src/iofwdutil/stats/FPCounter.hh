#ifndef SRC_IOFWDUTIL_STATS_FPCOUNTER_HH
#define SRC_IOFWDUTIL_STATS_FPCOUNTER_HH

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

class FPCounter
{
    public:
        FPCounter(const std::string & name, double data, bool invert, bool run=true) : cv_(name + std::string("_fp"), std::string("%f")), run_(run), val_(data), invert_(invert)
        {
            if(run)
            {
                if(!invert)
                {
                    cv_.get() += val_;
                }
                else
                {
                    cv_.get() += (1.0 / val_);
                }
            }
            else
            {
                cv_.get() = 0.0;
            }
        }

        ~FPCounter()
        {
        }

        double curValue()
        {
            return cv_.get();
        }

        void operator+=(FPCounter a)
        {
            if(!invert_)
            {
                val_ += a.val_;
            }
            else
            {
                val_ += (1.0 / a.val_);
            }
        }

        double toDouble()
        {
            return static_cast<double>(val_);
        }

    protected:

        Counter< double > cv_;
        bool run_;
        double val_;
        bool invert_;
};

    } /* stats */
} /* iofwdutil */
#endif
