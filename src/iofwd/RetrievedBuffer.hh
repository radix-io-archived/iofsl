#ifndef __IOFWD_RETRIEVEDBUFFER_HH__
#define __IOFWD_RETRIEVEDBUFFER_HH__

#include "iofwd/BMIBufferPool.hh"

namespace iofwd
{

struct RetrievedBuffer
{
    iofwd::BMIBufferWrapper * buffer;
    uint64_t siz;
    uint64_t off;

    // for async request
    const char ** mem_starts;
    size_t * mem_sizes;
    uint64_t * file_starts;
    uint64_t * file_sizes;
    int * ret;

    RetrievedBuffer()
        : buffer(NULL), siz(0), off(0), mem_starts(NULL), mem_sizes(NULL),
          file_starts(NULL), file_sizes(NULL), ret(NULL)
    {
    }

    ~RetrievedBuffer()
    {
        cleanup();
    }

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
        {
            delete buffer;
            buffer = NULL;
        }
        if(mem_starts)
        {
            delete [] mem_starts;
            mem_starts = NULL;
        }
        if(mem_sizes)
        {
            delete [] mem_sizes;
            mem_sizes = NULL;
        }
        if(file_starts)
        {
            delete [] file_starts;
            file_starts = NULL;
        }
        if(file_sizes)
        {
            delete [] file_sizes;
            file_sizes = NULL;
        }
        if(ret)
        {
            delete ret;
            ret = NULL;
        }

        siz = 0;
        off = 0;
    }
};

}

#endif
