#ifndef IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH
#define IOFWD_TASKSM_SMRETRIEVEDBUFFER_HH

#include "iofwd/BMIMemoryManager.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

struct SMRetrievedBuffer
{
    BMIMemoryAlloc * buffer;
    zoidfs::zoidfs_file_size_t siz;
    size_t p_siz;
    zoidfs::zoidfs_file_ofs_t off;

    // for async request
    const char ** mem_starts;
    size_t * mem_sizes;
    const zoidfs::zoidfs_file_ofs_t * file_starts;
    const zoidfs::zoidfs_file_size_t * file_sizes;
    int * ret;
    iofwdutil::bmi::BMIAddr addr_;
    iofwdutil::bmi::BMI::AllocType allocType_;

    SMRetrievedBuffer(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t p_size);
    ~SMRetrievedBuffer();

    /* before this object is used / reused  reinit it */
    void reinit()
    {
        /* cleanup any old data stuctures */
        cleanup();

        /* allocate a new BMIBufferWrapper */
        buffer = new BMIMemoryAlloc(addr_, allocType_, p_siz);
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
