#ifndef SRC_IOFWDUTIL_STATS_BYTECOUNTERWINDOW_HH
#define SRC_IOFWDUTIL_STATS_BYTECOUNTERWINDOW_HH

#include <string>
#include "iofwdutil/stats/CounterWindow.hh"
#include "iofwdutil/stats/CounterHelper.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
    namespace stats
    {
template<int N>
class ByteCounterWindow : public CounterWindow<uint64_t, N>,
    public CounterHelper< ByteCounterWindow<N> >
{
    protected:
        friend class CounterHelper< ByteCounterWindow<N> >;

        ByteCounterWindow(std::string name) :
            CounterWindow<uint64_t, N>(name + "_byte")
        {
        }

        virtual ~ByteCounterWindow()
        {
        }
};

    } /* stats */
} /* iofwdutil */
#endif
