#include "iofwdutil/PerfCounters.hh"

namespace iofwdutil
{
    /* counter create and destroy */
    int PerfCounterAdd(void ** pc_tree, char * pc_key, iofwd_pc_dt_t pc_dt)
    {
        return perf_counters_counter_add(pc_tree, pc_key, pc_dt);
    }
    
    int PerfCounterDelete(void ** pc_tree, char * pc_key)
    {
        return perf_counters_counter_delete(pc_tree, pc_key);
    }

    int PerfCounterCleanup(void * pc_tree)
    {
        return perf_counters_cleanup(pc_tree);
    }

    /* counter updates */
    int PerfCounterCounterUpdate(void ** pc_tree, char * pc_key, void * pc_data)
    {
        return perf_counters_counter_update(pc_tree, pc_key, pc_data);
    }

    int PerfCounterReset(void ** pc_tree, char * pc_key)
    {
        return perf_counters_counter_reset(pc_tree, pc_key);
    }

    int PerfCounterGet(void ** pc_tree, char * pc_key, void * pc_data)
    {
        return perf_counters_counter_get(pc_tree, pc_key, pc_data);
    }
}
