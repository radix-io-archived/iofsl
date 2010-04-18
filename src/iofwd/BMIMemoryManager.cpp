#include "iofwd/BMIMemoryManager.hh"

#include <cassert>

using namespace std;

namespace iofwd
{

BMIMemoryAlloc::BMIMemoryAlloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t bufferSize)
    : allocated_(false), numTokens_(0), bufferSize_(bufferSize), memory_(NULL), addr_(addr), allocType_(allocType)
{
}

BMIMemoryAlloc::~BMIMemoryAlloc()
{
    /* if we allocated a buffer, dealloc it */
    if(allocated_)
        dealloc();
}

/* is the buffer allocated? */
bool BMIMemoryAlloc::allocated() const
{
    return allocated_;
}

/* get the memory buffer start */
void * BMIMemoryAlloc::getMemory() const
{
    return memory_->get();
}

/* get the bmi memory buffer */
iofwdutil::bmi::BMIBuffer * BMIMemoryAlloc::getBMIBuffer() const
{
    return memory_;
}

/* get the memory buffer start */
size_t BMIMemoryAlloc::getMemorySize() const
{
    return memory_->size();
}

/* get the number of tokens held by this alloc */
size_t BMIMemoryAlloc::getNumTokens() const
{
    return numTokens_;
}

void BMIMemoryAlloc::alloc(int numTokens)
{
    /* set the memory params */
    allocated_ = true;
    numTokens_ = numTokens;

    /* create the memory buffer */
    memory_ = new iofwdutil::bmi::BMIBuffer(addr_, allocType_);
    
    /* resize the buffer to bufferSize_ */
    memory_->resize(bufferSize_);
}

void BMIMemoryAlloc::dealloc()
{
    /* remove the BMI memory buffer */
    if(memory_)
        delete memory_;

    /* reset the memory params */
    allocated_ = false;
    numTokens_ = 0;
    memory_ = NULL;
}

/*
 * This function is a callback that is passed to the memory manager token
 *  resource. In addition to calling the user callback, it will allocate
 *  the bmi buffer once the token was granted. This is basically
 *  a wrapper around the callback with some additional memory manager
 *  ops. 
 */
void BMIMemoryManager::runBufferAllocCB(int status, BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
    /* have the buffer wrapper allocate the buffer and consume one token */
    memAlloc->alloc(1);

    /* invoke the callback */
    cb(status);
}

/*
 * Setup the memory manager.
 *  - get the pipeline size (config file) 
 *  - get the number of concurrent buffers allocs (config file)
 *  - create the token resource
 *  - start the token resource
 */
BMIMemoryManager::BMIMemoryManager(const iofwdutil::ConfigFile & c)
{
    /* get params from the config file */
    pipelineSize_ = c.getKeyAsDefault("buffersize", 0);
    int numTokens = c.getKeyAsDefault("maxnumbuffers", 0);

    /* create the token resource */
    tokens_ = new iofwdevent::TokenResource(numTokens); 

    /* start the token resource */
    tokens_->start();
}

/*
 * On destruction, cleanup up the token resource
 */
BMIMemoryManager::~BMIMemoryManager()
{
    /* stop and delete the token resource */
    if(tokens_)
    {
        tokens_->stop();
        delete tokens_;
    }
}

/*
 * This is the public buffer allocation method
 */
void BMIMemoryManager::alloc(iofwdevent::CBType cb, BMIMemoryAlloc * memAlloc)
{
    /* construct the mem manager callback */
    boost::function<void(int)> bmmCB = boost::bind(&iofwd::BMIMemoryManager::runBufferAllocCB, this, _1, memAlloc, cb);

    /* get the tokens */
    tokens_->request(boost::bind(bmmCB, 0), 1);
}

/*
 * This is the public buffer deallocation method
 */
void BMIMemoryManager::dealloc(BMIMemoryAlloc * memAlloc)
{
    /* free the BMI memory */
    memAlloc->dealloc();

    /* return the tokens */
    tokens_->release(1);
}

}
