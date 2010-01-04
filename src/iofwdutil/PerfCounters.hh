#ifndef __IOFWDUTIL_PERF_COUNTERS_HH__
#define __IOFWDUTIL_PERF_COUNTERS_HH__

#include "c-util/perf-counters.h"

namespace iofwdutil
{
    /* counter create and destroy */
    int PerfCounterAdd(void ** pc_tree, char * pc_key, iofwd_pc_dt_t pc_dt);
    int PerfCounterDelete(void ** pc_tree, char * pc_key);
    int PerfCounterCleanup(void * pc_tree);

    /* counter updates */
    int PerfCounterCounterUpdate(void ** pc_tree, char * pc_key, void * pc_data);
    int PerfCounterReset(void ** pc_tree, char * pc_key);
    int PerfCounterGet(void ** pc_tree, char * pc_key, void * pc_data);
}

#endif
