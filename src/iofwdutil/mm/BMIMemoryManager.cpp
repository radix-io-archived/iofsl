#include "iofwdutil/mm/BMIMemoryManager.hh"
#include "iofwdutil/tools.hh"

#include <cassert>

using namespace std;

namespace iofwdutil
{
    namespace mm
    {

BMIMemoryAlloc::BMIMemoryAlloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t bufferSize)
    : IOFWDMemoryAlloc(), allocated_(false), numTokens_(0), bufferSize_(bufferSize), memory_(NULL), addr_(addr), allocType_(allocType), cb_count_(2)
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

size_t BMIMemoryAlloc::getReqMemorySize() const
{
    return bufferSize_;
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
 *  the bmi buffer once the token was granted. This is basically
 *  a wrapper around the callback with some additional memory manager
 *  ops. 
 */
void BMIMemoryManager::runBufferAllocCB1(int UNUSED(status), BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
    boost::function<void(int)>bmmCB = boost::bind(&iofwdutil::mm::BMIMemoryManager::runBufferAllocCB2, this, _1, dynamic_cast<BMIMemoryAlloc *>(memAlloc), cb);
    mem_->request(bmmCB, memAlloc->getReqMemorySize());
}

void BMIMemoryManager::runBufferAllocCB2(int status, BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
    /* have the buffer wrapper allocate the buffer and consume one token */
    memAlloc->alloc(1);
    cb(status);
}

/*
 * Setup the memory manager.
 *  - get the number of concurrent buffers allocs (config file)
 *  - create the token resource
 *  - start the token resource
 */
BMIMemoryManager::BMIMemoryManager()
{
}

/* static variables for the mem manager */
int iofwdutil::mm::BMIMemoryManager::numTokens_ = 0;
size_t iofwdutil::mm::BMIMemoryManager::memAmount_ = 0;
boost::mutex iofwdutil::mm::BMIMemoryManager::bmm_setup_mutex_;

void BMIMemoryManager::setMaxNumBuffers(int numTokens)
{
    boost::mutex::scoped_lock lock(bmm_setup_mutex_);
    numTokens_ = numTokens;
}

void BMIMemoryManager::setMaxMemAmount(size_t mem)
{
    boost::mutex::scoped_lock lock(bmm_setup_mutex_);
    memAmount_ = mem;
}

void BMIMemoryManager::start()
{
    /* create the token resource */
    tokens_ = new iofwdevent::TokenResource(numTokens_); 
    mem_ = new iofwdevent::TokenResource(memAmount_); 

    /* start the token resource */
    tokens_->start();
    mem_->start();
}

void BMIMemoryManager::reset()
{
    if(tokens_)
    {
        /* stop the token resource */
        tokens_->stop();
        mem_->stop();

        /* delete the token object */
        delete tokens_;
        tokens_ = NULL;

        delete mem_;
        mem_ = NULL;
    }
}

/*
 * On destruction, cleanup up the token resource
 */
BMIMemoryManager::~BMIMemoryManager()
{
    /* stop and delete the token resource */
    reset();
}

/*
 * This is the public buffer allocation method
 */
void BMIMemoryManager::alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc)
{
    /* construct the mem manager callback */
    boost::function<void(int)>bmmCB = boost::bind(&iofwdutil::mm::BMIMemoryManager::runBufferAllocCB1, this, _1, dynamic_cast<BMIMemoryAlloc *>(memAlloc), cb);

    /* get the tokens */
    tokens_->request(boost::bind(bmmCB, 0), 1);
}

/*
 * This is the public buffer deallocation method
 */
void BMIMemoryManager::dealloc(IOFWDMemoryAlloc * memAlloc)
{
    /* free the BMI memory */
    size_t mr = dynamic_cast<BMIMemoryAlloc *>(memAlloc)->getReqMemorySize();
    memAlloc->dealloc();

    /* return the tokens */
    tokens_->release(1);
    mem_->release(mr);
}

bool BMIMemoryManager::try_alloc(IOFWDMemoryAlloc * memAlloc)
{
    bool req = false;
 
    req = tokens_->try_request(1);

    /* if we got the buffer token, try to get the mem token*/
    if(req)
    {
        req = mem_->try_request(dynamic_cast<BMIMemoryAlloc *>(memAlloc)->getReqMemorySize());
        /* we got the buffer space, so return */
        if(req)
        {
            dynamic_cast<BMIMemoryAlloc *>(memAlloc)->alloc(1);
            return true;
        }
        /* we could not get the buffer, so release the buffer token */
        else
        {
            tokens_->release(1);
        }
    }
    
    return false;
}
    }
}
