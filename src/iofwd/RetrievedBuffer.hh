#ifndef IOFWD_RETRIEVEDBUFFER_HH
#define IOFWD_RETRIEVEDBUFFER_HH

#include "iofwdutil/mm/IOFWDMemoryManager.hh"

namespace iofwd
{

struct RetrievedBuffer
{
    zoidfs::zoidfs_file_size_t siz;
    size_t p_siz;
    zoidfs::zoidfs_file_ofs_t off;

    // for async request
    const char ** mem_starts;
    size_t * mem_sizes;
    zoidfs::zoidfs_file_ofs_t * file_starts;
    zoidfs::zoidfs_file_size_t * file_sizes;
    int * ret;
    iofwdutil::mm::IOFWDMemoryAlloc * buffer_;

    RetrievedBuffer(size_t p_size)
        : siz(0), p_siz(p_size), off(0), mem_starts(NULL), mem_sizes(NULL),
          file_starts(NULL), file_sizes(NULL), ret(NULL), buffer_(NULL)
    {
    }

    ~RetrievedBuffer()
    {
        cleanup();
    }

    void cleanup()
    {
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

    size_t getsize()
    {
        return p_siz;
    }

    void * getptr()
    {
        return buffer_->getMemory();
    }
};

}

#endif
