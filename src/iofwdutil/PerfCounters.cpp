#include "iofwdutil/PerfCounters.hh"

#include <cstdio>

namespace iofwdutil
{
    /* set the perf counters collection to NULL */
    PerfCounters * PerfCounters::perf_counters_instance_ = NULL;
    PerfCountersDestroyer PerfCounters::perf_counters_destroyer_instance_;

    /* return an instance of the PerfCounters */
    PerfCounters * PerfCounters::Instance()
    {
        /* only create the object once */
        if(!perf_counters_instance_)
        {
            perf_counters_instance_ = new PerfCounters();
            perf_counters_destroyer_instance_.SetPerfCounters(perf_counters_instance_);
        }

        return perf_counters_instance_;
    }
    
    PerfCounters::PerfCounters()
    {
    }

    /* cleanup if an instance of the object was created */
    PerfCounters::~PerfCounters()
    {
    }
}
