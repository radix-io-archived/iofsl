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
#include "iofwdutil/mm/IOFWDMemoryManager.hh"
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
 *
 *  @TODO: Make this into a service that depends on BMI?
 */
namespace iofwdutil
{
    namespace mm
    {

/* forward decls */
class BMIMemoryManager;

/* wrapper for a single BMI memory allocation from the BMIMemoryManager*/
class BMIMemoryAlloc : public IOFWDMemoryAlloc
{
    public:
        BMIMemoryAlloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t bufferSize_);

        ~BMIMemoryAlloc();

        /* is the buffer allocated? */
        virtual bool allocated() const;

        /* get the memory buffer start */
        virtual void * getMemory() const;

        /* get the bmi memory buffer */
        iofwdutil::bmi::BMIBuffer * getBMIBuffer() const;

        /* get the memory buffer start */
        virtual size_t getMemorySize() const;

        size_t getReqMemorySize() const;

        /* get the number of tokens held by this alloc */
        virtual size_t getNumTokens() const;

        virtual void alloc(int numTokens);
        virtual void dealloc();

        bmi_buffer_type bmiType()
        {
            return memory_->bmiType();
        }

        int decgetCBCount()
        {
            boost::mutex::scoped_lock lock(cbcount_mutex_);
            cb_count_--;
            return cb_count_;
        }

    protected:
        /* friends that can invoke the alloc and dealloc methods */
        friend class BMIMemoryManager;

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

        boost::mutex cbcount_mutex_;
        int cb_count_;
};

/* allocation manager for BMI buffers */
class BMIMemoryManager : public IOFWDMemoryManager, public iofwdutil::Singleton < BMIMemoryManager > 
{
    public:
        BMIMemoryManager();
        ~BMIMemoryManager();
        void setup(const iofwdutil::ConfigFile & c);

        /* submit a buffer request */
        void alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc_);
        bool try_alloc(IOFWDMemoryAlloc * memAlloc);

        /* return a buffer to the manager */
        void dealloc(IOFWDMemoryAlloc * memAlloc_);

        BMIMemoryAlloc * remove(void * ptr);

        /* start the memory manager */
        void start();

        /* reset the memory manager */
        void reset();

        /* memory manager setup */
        static void setMaxNumBuffers(int numBuffers);
        static void setWarnNumBuffers(int numTokens);
        static void setMaxMemAmount(size_t mem);
        static void setMemWarnAmount(size_t mem);

    protected:
        void runBufferAllocCB1(iofwdevent::CBException status,
              BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb);

        void runBufferAllocCB2(iofwdevent::CBException status,
              BMIMemoryAlloc * memAlloc, iofwdevent::CBType cb);

        /*
         * We use a TokenResource to limit the number of buffers consumed by
         * the server Currently, we do a 1 to 1 mapping of a token to a buffer
         * of pipelineSize_.
         *
         * @TODO: Come up with a better token <-> buffer scheme... not every
         * transfer will neeed pipelineSize_ sized buffers 
         */
        iofwdevent::TokenResource * tokens_;
        iofwdevent::TokenResource * mem_;

        static int numTokens_;
        static size_t memAmount_;
        static int warnNumTokens_;
        static size_t memWarnAmount_;
        static boost::mutex bmm_setup_mutex_;

        std::vector<BMIMemoryAlloc *> alloc_list_;
        boost::mutex bmm_tracker_mutex_;
};
    }
}

#endif
