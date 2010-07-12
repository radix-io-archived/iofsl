#ifndef IOFWD_HEAP_MEMORY_MANAGER_HH
#define IOFWD_HEAP_MEMORY_MANAGER_HH

#include <deque>
#include <boost/thread.hpp>
#include "iofwdutil/ConfigFile.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdutil/Singleton.hh"
#include "boost/thread/mutex.hpp"
#include "iofwdutil/mm/IOFWDMemoryManager.hh"

namespace iofwdutil
{
    namespace mm
    {

/* forward decls */
class HeapMemoryManager;

/* wrapper for a single Heap memory allocation from the HeapMemoryManager*/
class HeapMemoryAlloc : public IOFWDMemoryAlloc
{
    public:
        HeapMemoryAlloc(size_t bufferSize_);

        ~HeapMemoryAlloc();

        /* is the buffer allocated? */
        virtual bool allocated() const;

        /* get the memory buffer start */
        virtual void * getMemory() const;

        /* get the memory buffer start */
        virtual size_t getMemorySize() const;

        /* get the number of tokens held by this alloc */
        virtual size_t getNumTokens() const;

        virtual void alloc(int numTokens);
        virtual void dealloc();

    protected:
        /* friends that can invoke the alloc and dealloc methods */
        friend class HeapMemoryManager;

        /* allocation valid flag */
        bool allocated_;

        /* number of tokens owned by this alloc */
        size_t numTokens_;
        size_t bufferSize_;

        /* the memory buffer */
        char * memory_;
};

/* allocation manager for Heap buffers */
class HeapMemoryManager : public IOFWDMemoryManager, public iofwdutil::Singleton < HeapMemoryManager > 
{
    public:
        HeapMemoryManager();
        ~HeapMemoryManager();
        void setup(const iofwdutil::ConfigFile & c);

        /* submit a buffer request */
        void alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc_);

        /* return a buffer to the manager */
        void dealloc(IOFWDMemoryAlloc * memAlloc_);

        /* start the memory manager */
        void start();

        /* reset the memory manager */
        void reset();

        /* memory manager setup */
        static void setMaxNumBuffers(int numBuffers);

    protected:
        void runBufferAllocCB(int status, HeapMemoryAlloc * memAlloc, iofwdevent::CBType cb);        

        /*
         * We use a TokenResource to limit the number of buffers consumed by the server
         *  Currently, we do a 1 to 1 mapping of a token to a buffer of pipelineSize_.
         *  @TODO: Come up with a better token <-> buffer scheme...
         */
        iofwdevent::TokenResource * tokens_;

        static int numTokens_;
        static boost::mutex hmm_setup_mutex_;
};
    }
}

#endif
