#ifndef SRC_IOFWDUTIL_STATS_TIMECOUNTERWINDOW_HH
#define SRC_IOFWDUTIL_STATS_TIMECOUNTERWINDOW_HH

#include <string>
#include "iofwdutil/stats/CounterWindow.hh"
#include "iofwdutil/stats/CounterHelper.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
    namespace stats
    {
template<int N>
class TimeCounterWindow : public CounterWindow<double, N>,
    public CounterHelper< TimeCounterWindow<N> >
{
    public:
        double start()
        {
            return getCurTime();
        }

        double stop()
        {
            return getCurTime();
        }

    protected:
        friend class CounterHelper< TimeCounterWindow<N> >;

        TimeCounterWindow(std::string name) :
            CounterWindow<double, N>(name + "_time")
        {
        }

        virtual ~TimeCounterWindow()
        {
        }

        double getCurTime()
        {
            struct timespec tp;
            assert(clock_gettime(CLOCK_REALTIME, &tp) == 0);
            return TimeVal(tp.tv_sec, tp.tv_nsec).toDouble();
        }
};

    } /* stats */
} /* iofwdutil */
#endif
