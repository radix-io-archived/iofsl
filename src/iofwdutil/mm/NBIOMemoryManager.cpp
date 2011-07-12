#include "iofwdutil/mm/NBIOMemoryManager.hh"
#include "iofwdutil/mm/BMIMemoryManager.hh"
#include "iofwdutil/tools.hh"

#include <cassert>

using namespace std;

namespace iofwdutil
{
    namespace mm
    {

NBIOMemoryAlloc::NBIOMemoryAlloc(size_t bufferSize)
    : IOFWDMemoryAlloc(),
    allocated_(false),
    numTokens_(0),
    bufferSize_(bufferSize),
    memory_(NULL),
    cb_count_(2)
{
}

NBIOMemoryAlloc::~NBIOMemoryAlloc()
{
    /* if we allocated a buffer, dealloc it */
    if(allocated_)
        dealloc();
}

/* is the buffer allocated? */
bool NBIOMemoryAlloc::allocated() const
{
    return allocated_;
}

/* get the memory buffer start */
void * NBIOMemoryAlloc::getMemory() const
{
    return memory_;
}

void NBIOMemoryAlloc::setMemory(void * m)
{
    memory_ = m;
}

/* get the memory buffer start */
size_t NBIOMemoryAlloc::getMemorySize() const
{
    return bufferSize_;
}

size_t NBIOMemoryAlloc::getReqMemorySize() const
{
    return bufferSize_;
}

/* get the number of tokens held by this alloc */
size_t NBIOMemoryAlloc::getNumTokens() const
{
    return numTokens_;
}

void NBIOMemoryAlloc::alloc(int numTokens)
{
    /* set the memory params */
    allocated_ = true;
    numTokens_ = numTokens;

    /* create the memory buffer */
    if(iofwdutil::mm::NBIOMemoryManager::zeroCopy())
    {
        memory_ = NULL;
    }
    else
    {
        memory_ = new char[bufferSize_];
    }
}

void NBIOMemoryAlloc::dealloc()
{
    /* remove the NBIO memory buffer */
    if(iofwdutil::mm::NBIOMemoryManager::zeroCopy())
    {
        /* TODO generalize this */
        BMIMemoryAlloc * a = iofwdutil::mm::BMIMemoryManager::instance().remove(memory_);

        /* cleanup */
        iofwdutil::mm::BMIMemoryManager::instance().dealloc(a);
        delete a;

        memory_ = NULL;
    }
    else
    {
        if(memory_)
        {
            delete [] static_cast<char *>(memory_);
            memory_ = NULL;
        }
    }

    /* reset the memory params */
    allocated_ = false;
    numTokens_ = 0;
}

void NBIOMemoryManager::runBufferAllocCB1(iofwdevent::CBException status,
      NBIOMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
   status.check ();

   iofwdevent::CBType nbiommCB =
      boost::bind(&iofwdutil::mm::NBIOMemoryManager::runBufferAllocCB2, this,
            _1, dynamic_cast<NBIOMemoryAlloc *>(memAlloc), cb);
   mem_->request(nbiommCB, memAlloc->getReqMemorySize());
}

void NBIOMemoryManager::runBufferAllocCB2(iofwdevent::CBException status,
      NBIOMemoryAlloc * memAlloc, iofwdevent::CBType cb)
{
   status.check ();

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
NBIOMemoryManager::NBIOMemoryManager()
{
}

/* static variables for the mem manager */
int iofwdutil::mm::NBIOMemoryManager::numTokens_ = 0;
int iofwdutil::mm::NBIOMemoryManager::warnNumTokens_ = 0;
size_t iofwdutil::mm::NBIOMemoryManager::memAmount_ = 0;
size_t iofwdutil::mm::NBIOMemoryManager::memWarnAmount_ = 0;
bool iofwdutil::mm::NBIOMemoryManager::zeroCopy_ = false;
boost::mutex iofwdutil::mm::NBIOMemoryManager::nbiomm_setup_mutex_;

void NBIOMemoryManager::setMaxNumBuffers(int numTokens)
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    numTokens_ = numTokens;
}

void NBIOMemoryManager::setWarnNumBuffers(int numTokens)
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    warnNumTokens_ = numTokens;
}

void NBIOMemoryManager::setMaxMemAmount(size_t mem)
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    memAmount_ = mem;
}

void NBIOMemoryManager::setMemWarnAmount(size_t mem)
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    memWarnAmount_ = mem;
}

bool NBIOMemoryManager::zeroCopy()
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    return zeroCopy_;
}

void NBIOMemoryManager::disableZeroCopy()
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    zeroCopy_ = false;
}

void NBIOMemoryManager::enableZeroCopy()
{
    boost::mutex::scoped_lock lock(nbiomm_setup_mutex_);
    zeroCopy_ = true;
}

void NBIOMemoryManager::start()
{
    /* create the token resource */
    tokens_ = new iofwdevent::TokenResource(numTokens_, warnNumTokens_,
            std::string("NBIO Buffer Tokens: ")); 
    mem_ = new iofwdevent::TokenResource(memAmount_, memWarnAmount_,
            std::string("NBIO Mem Alloc Tokens: ")); 

    /* start the token resource */
    tokens_->start();
    mem_->start();
}

void NBIOMemoryManager::reset()
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
NBIOMemoryManager::~NBIOMemoryManager()
{
    /* stop and delete the token resource */
    reset();
}

/*
 * This is the public buffer allocation method
 */
void NBIOMemoryManager::alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc)
{
    /* construct the mem manager callback */
   iofwdevent::CBType nbiommCB =
      boost::bind(&iofwdutil::mm::NBIOMemoryManager::runBufferAllocCB1, this,
            _1, dynamic_cast<NBIOMemoryAlloc *>(memAlloc), cb);

    /* get the tokens */
    tokens_->request(nbiommCB, 1);
}

/*
 * This is the public buffer deallocation method
 */
void NBIOMemoryManager::dealloc(IOFWDMemoryAlloc * memAlloc)
{
    /* free the NBIO memory */
    size_t mr = dynamic_cast<NBIOMemoryAlloc *>(memAlloc)->getReqMemorySize();
    memAlloc->dealloc();

    /* return the tokens */
    tokens_->release(1);
    mem_->release(mr);
}

bool NBIOMemoryManager::try_alloc(IOFWDMemoryAlloc * memAlloc)
{
    bool req = false;
 
    req = tokens_->try_request(1);

    /* if we got the buffer token, try to get the mem token*/
    if(req)
    {
        req = mem_->try_request(dynamic_cast<NBIOMemoryAlloc
                *>(memAlloc)->getReqMemorySize());

        /* we got the buffer space, so return */
        if(req)
        {
            dynamic_cast<NBIOMemoryAlloc *>(memAlloc)->alloc(1);
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
