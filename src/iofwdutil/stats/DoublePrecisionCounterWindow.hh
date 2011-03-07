#ifndef SRC_IOFWDUTIL_STATS_DOUBLEPRECISIONCOUNTERWINDOW_HH
#define SRC_IOFWDUTIL_STATS_DOUBLEPRECISIONCOUNTERWINDOW_HH

#include <string>
#include "iofwdutil/stats/CounterWindow.hh"
#include "iofwdutil/stats/CounterHelper.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
    namespace stats
    {
template<int N>
class DoublePrecisionCounterWindow : public CounterWindow<double, N>,
    public CounterHelper< DoublePrecisionCounterWindow<N> >
{
    protected:
        friend class CounterHelper< DoublePrecisionCounterWindow<N> >;

        DoublePrecisionCounterWindow(std::string name) :
            CounterWindow<double, N>(name + "_double")
        {
        }

        virtual ~DoublePrecisionCounterWindow()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
