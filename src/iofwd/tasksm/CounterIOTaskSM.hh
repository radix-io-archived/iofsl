#ifndef IOFWD_TASKSM_COUNTERIOTASKSM_HH
#define IOFWD_TASKSM_COUNTERIOTASKSM_HH

#include "iofwdutil/stats/IncCounter.hh"
template<typename S>
class CounterIOTaskSM
{
    public:
        CounterIOTaskSM()
        {
        }

        ~CounterIOTaskSM()
        {
        }

        /* inc on task completion */
        iofwdutil::stats::IncCounter inc_counter_;
};

#endif
