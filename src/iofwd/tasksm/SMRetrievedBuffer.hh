#ifndef __IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH__
#define __IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH__

#include "iofwd/BMIBufferPool.hh"

namespace iofwd
{
    namespace tasksm
    {

struct SMRetrievedBuffer
{
    BMIBufferWrapper * buffer;
    uint64_t siz;
    uint64_t off;

    // for async request
    const char ** mem_starts;
    size_t * mem_sizes;
    uint64_t * file_starts;
    uint64_t * file_sizes;
    int * ret;

    SMRetrievedBuffer();
    ~SMRetrievedBuffer();

    /* before this object is used / reused  reinit it */
    void reinit()
    {
        /* cleanup any old data stuctures */
        cleanup();

        /* allocate a new BMIBufferWrapper */
        buffer = new BMIBufferWrapper();
    }

    void cleanup()
    {
        if(buffer)
            delete buffer;
        if(mem_starts)
            delete [] mem_starts;
        if(mem_sizes)
            delete [] mem_sizes;
        if(file_starts)
            delete [] file_starts;
        if(file_sizes)
            delete [] file_sizes;
        if(ret)
            delete ret;

        siz = 0;
        off = 0;
    }
};

    }
}

#endif
