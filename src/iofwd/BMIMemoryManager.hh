#ifndef IOFWD_BMI_MEMORY_MANAGER_HH
#define IOFWD_BMI_MEMORY_MANAGER_HH

#include <deque>
#include <boost/thread.hpp>
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdutil/Singleton.hh"
#include "boost/thread/mutex.hpp"

/*
 * The following classes are replacements for the BMIMemoryPool and MemoryPool
 *  classes. The old memory pools weren't really memory pools... more like
 *  managers. Aditionally, the old classes did not work well when there
 *  was high request pressure on the pools. So, the name is changed to reflect 
 *  the class capability and a different operatinonal mode / interface was 
 *  implemented.
 *
 * The goal for these classes is to provide an interface for tasks and SMs
 *  to allocate BMI memory. The memory manager uses a token resource to
 *  manage access to a finite / user-defined number of memory buffers. 
 *  Tasks setup up BMI memory requests through the allocation wrappers. These
 *  requests / wrappers are passed to the memory manager through the callback
 *  mechanism. When the callback is invoked (signaling that memory is available), 
 *  the memory allocation wrapper is initialized with the proper memory buffer. The
 *  memory allocation wrapper is initialized by the memory manager when the following
 *  conditions occur:
 *   1) the memory manager has a pending memory allocation request
 *   2) a free buffer is available
 *   3) the request has been granted (the memory alloc owns a token)
 * 
 * To use these classes, tasks and SMs do the following:
 *  1) Setup a BMIMemoryAlloc requests that map to
 *     the buffer requests needed
 *  2) When ready to allocate the buffer, the tasks pass
 *     BMIMemoryAlloc request to the BMIMemoryManager using alloc()
 *  3) When a token / buffer is availble, the BMIMmemoryManager
 *     assigns a token to the the BMIMmeoryAlloc wrapper and
 *     allocate the buffer
 *  4) When through with the buffer, the dealloc() method is called.
 *     This will free the buffer and give the token back to the 
 *     BMIMemoryManager.
 *
 * @TODO: This is a work in progress and is an initial replacement for the old
 *  memory pools. A long term goal is setup a server wide memory manager to
 *  track all memory usage (not just BMI memory usage). The BMIMemoryManager
 *  should be a specialized version of the general memory manager. The token
 *  management could be improved as well as the interface.
 */
namespace iofwd
{

/* forward decls */
class BMIMemoryManager;

/* wrapper for a single BMI memory allocation from the BMIMemoryManager*/
class BMIMemoryAlloc
{
    public:
        BMIMemoryAlloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t bufferSize_);

        ~BMIMemoryAlloc();

        /* is the buffer allocated? */
        bool allocated() const;

        /* get the memory buffer start */
        void * getMemory() const;

        /* get the bmi memory buffer */
        iofwdutil::bmi::BMIBuffer * getBMIBuffer() const;

        /* get the memory buffer start */
        size_t getMemorySize() const;

        /* get the number of tokens held by this alloc */
        size_t getNumTokens() const;

    protected:
        /* friends that can invoke the alloc and dealloc methods */
        friend class BMIMemoryManager;

        void alloc(int numTokens);
        void dealloc();

        /* allocation valid flag */
        bool allocated_;

        /* number of tokens owned by this alloc */
        size_t numTokens_;
        size_t bufferSize_;

        /* the memory buffer */
        iofwdutil::bmi::BMIBuffer * memory_;

        /* BMI params */
        iofwdutil::bmi::BMIAddr addr_;
        iofwdutil::bmi::BMI::AllocType allocType_;
};

/* allocation manager for BMI buffers */
class BMIMemoryManager : public iofwdutil::Singleton < BMIMemoryManager > 
{
    public:
        BMIMemoryManager();
        ~BMIMemoryManager();
        void setup(const iofwdutil::ConfigFile & c);

        /* submit a buffer request */
        void alloc(iofwdevent::CBType cb, BMIMemoryAlloc * memAlloc_);

        /* return a buffer to the manager */
        void dealloc(BMIMemoryAlloc * memAlloc_);

        /* start the memory manager */
        void start();

        /* reset the memory manager */
        void reset();

        /* memory manager setup */
        static void setMaxNumBuffers(int numBuffers);

    protected:
        void runBufferAllocCB(int status, BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb);        

        /*
         * We use a TokenResource to limit the number of buffers consumed by the server
         *  Currently, we do a 1 to 1 mapping of a token to a buffer of pipelineSize_.
         *  @TODO: Come up with a better token <-> buffer scheme... not every transfer will
         *  neeed pipelineSize_ sized buffers 
         */
        iofwdevent::TokenResource * tokens_;

        static int numTokens_;
        static boost::mutex bmm_setup_mutex_;
};

}

#endif
