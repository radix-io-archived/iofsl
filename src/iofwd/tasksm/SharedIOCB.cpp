#include "iofwd/tasksm/SharedIOCB.hh"

namespace iofwd
{
    namespace tasksm
    {
        SharedIOCB::~SharedIOCB()
        {
            /* delete all dyn allocated memory... allocated by the request scheduler */
            delete ret_;
            delete [] mem_starts_;
            delete [] mem_sizes_;
            delete [] file_starts_;
            delete [] file_sizes_;
        }

        MultiSharedIOCB::~MultiSharedIOCB()
        {
        }

        SingleSharedIOCB::~SingleSharedIOCB()
        {
        }
    }
}
