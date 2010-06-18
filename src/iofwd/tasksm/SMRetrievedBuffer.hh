#ifndef IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH
#define IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH

#include "iofwd/IOFWDMemoryManager.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/RetrievedBuffer.hh"

namespace iofwd
{
    namespace tasksm
    {

struct SMRetrievedBuffer : public iofwd::RetrievedBuffer
{
    IOFWDMemoryAlloc * buffer_;
    zoidfs::zoidfs_file_size_t siz;
    size_t p_siz;
    zoidfs::zoidfs_file_ofs_t off;

    // for async request
    const char ** mem_starts;
    size_t * mem_sizes;
    const zoidfs::zoidfs_file_ofs_t * file_starts;
    const zoidfs::zoidfs_file_size_t * file_sizes;
    int * ret;

    SMRetrievedBuffer(size_t p_size);
    ~SMRetrievedBuffer();

    void cleanup()
    {
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
