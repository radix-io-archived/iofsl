#include "iofwdutil/mm/HeapMemoryManager.hh"

#include <cassert>

using namespace std;

namespace iofwdutil
{
    namespace mm
    {

HeapMemoryAlloc::HeapMemoryAlloc(size_t bufferSize)
    : IOFWDMemoryAlloc(), allocated_(false), numTokens_(0), bufferSize_(bufferSize), memory_(NULL)
{
}

HeapMemoryAlloc::~HeapMemoryAlloc()
{
    /* if we allocated a buffer, dealloc it */
    if(allocated_)
        dealloc();
}

/* is the buffer allocated? */
bool HeapMemoryAlloc::allocated() const
{
    return allocated_;
}

/* get the memory buffer start */
void * HeapMemoryAlloc::getMemory() const
{
    return static_cast<void *>(memory_);
}

/* get the memory buffer start */
size_t HeapMemoryAlloc::getMemorySize() const
{
    return bufferSize_;
}

/* get the number of tokens held by this alloc */
size_t HeapMemoryAlloc::getNumTokens() const
{
    return numTokens_;
}

void HeapMemoryAlloc::alloc(int numTokens)
{
    /* set the memory params */
    allocated_ = true;
    numTokens_ = numTokens;

    /* create the memory buffer */
    memory_ = new char[bufferSize_];
}

void HeapMemoryAlloc::dealloc()
{
    /* remove the BMI memory buffer */
    if(memory_)
    {
        delete memory_;
        memory_ = NULL;

        /* reset the memory params */
        allocated_ = false;
        numTokens_ = 0;
    }
}

/*
 * This function is a callback that is passed to the memory manager token
 *  resource. In addition to calling the user callback, it will allocate
 *  the buffer once the token was granted. This is basically
 *  a wrapper around the callback with some additional memory manager
 *  ops. 
 */
void HeapMemoryManager::runBufferAllocCB(int status, HeapMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
    /* have the buffer wrapper allocate the buffer and consume one token */
    memAlloc->alloc(1);

    /* invoke the callback */
    cb(status);
}

/*
 * Setup the memory manager.
 *  - get the number of concurrent buffers allocs (config file)
 *  - create the token resource
 *  - start the token resource
 */
HeapMemoryManager::HeapMemoryManager()
{
}

/* static variables for the mem manager */
int iofwdutil::mm::HeapMemoryManager::numTokens_ = 0;
boost::mutex iofwdutil::mm::HeapMemoryManager::hmm_setup_mutex_;

void HeapMemoryManager::setMaxNumBuffers(int numTokens)
{
    boost::mutex::scoped_lock lock(hmm_setup_mutex_);
    numTokens_ = numTokens;
}

void HeapMemoryManager::start()
{
    /* create the token resource */
    tokens_ = new iofwdevent::TokenResource(numTokens_); 

    /* start the token resource */
    tokens_->start();
}

void HeapMemoryManager::reset()
{
    if(tokens_)
    {
        /* stop the token resource */
        tokens_->stop();

        /* delete the token object */
        delete tokens_;
        tokens_ = NULL;
    }
}

/*
 * On destruction, cleanup up the token resource
 */
HeapMemoryManager::~HeapMemoryManager()
{
    /* stop and delete the token resource */
    reset();
}

/*
 * This is the public buffer allocation method
 */
void HeapMemoryManager::alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc)
{
    /* construct the mem manager callback */
    boost::function<void(int)> hmmCB = boost::bind(&iofwdutil::mm::HeapMemoryManager::runBufferAllocCB, this, _1, dynamic_cast<HeapMemoryAlloc *>(memAlloc), cb);

    /* get the tokens */
    tokens_->request(boost::bind(hmmCB, 0), 1);
}

/*
 * This is the public buffer deallocation method
 */
void HeapMemoryManager::dealloc(IOFWDMemoryAlloc * memAlloc)
{
    /* free the memory */
    memAlloc->dealloc();

    /* return the tokens */
    tokens_->release(1);
}
    }
}
