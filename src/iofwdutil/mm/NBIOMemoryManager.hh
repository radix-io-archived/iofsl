#ifndef IOFWD_NBIO_MEMORY_MANAGER_HH
#define IOFWD_NBIO_MEMORY_MANAGER_HH

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
class NBIOMemoryManager;

/* wrapper for a single NBIO memory allocation from the NBIOMemoryManager*/
class NBIOMemoryAlloc : public IOFWDMemoryAlloc
{
    public:
        NBIOMemoryAlloc(size_t bufferSize_);

        ~NBIOMemoryAlloc();

        /* is the buffer allocated? */
        virtual bool allocated() const;

        /* get the memory buffer start */
        virtual void * getMemory() const;

        /* get the bmi memory buffer */
        void * getNBIOBuffer() const;

        /* get the memory buffer start */
        virtual size_t getMemorySize() const;

        size_t getReqMemorySize() const;

        /* get the number of tokens held by this alloc */
        virtual size_t getNumTokens() const;

        virtual void alloc(int numTokens);
        virtual void dealloc();

        int decgetCBCount()
        {
            boost::mutex::scoped_lock lock(cbcount_mutex_);
            cb_count_--;
            return cb_count_;
        }

    protected:
        /* friends that can invoke the alloc and dealloc methods */
        friend class NBIOMemoryManager;

        /* allocation valid flag */
        bool allocated_;

        /* number of tokens owned by this alloc */
        size_t numTokens_;
        size_t bufferSize_;

        /* the memory buffer */
        void * memory_;

        boost::mutex cbcount_mutex_;
        int cb_count_;
};

/* allocation manager for NBIO buffers */
class NBIOMemoryManager :
    public IOFWDMemoryManager,
    public iofwdutil::Singleton < NBIOMemoryManager > 
{
    public:
        NBIOMemoryManager();
        ~NBIOMemoryManager();
        void setup(const iofwdutil::ConfigFile & c);

        /* submit a buffer request */
        void alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc_);
        bool try_alloc(IOFWDMemoryAlloc * memAlloc);

        /* return a buffer to the manager */
        void dealloc(IOFWDMemoryAlloc * memAlloc_);

        /* start the memory manager */
        void start();

        /* reset the memory manager */
        void reset();

        /* memory manager setup */
        static void setMaxNumBuffers(int numBuffers);
        static void setWarnNumBuffers(int numBuffers);
        static void setMaxMemAmount(size_t mem);
        static void setMemWarnAmount(size_t mem);

    protected:
        void runBufferAllocCB1(iofwdevent::CBException status,
              NBIOMemoryAlloc * memAlloc, iofwdevent::CBType cb);

        void runBufferAllocCB2(iofwdevent::CBException status,
              NBIOMemoryAlloc * memAlloc, iofwdevent::CBType cb);

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
        static int warnNumTokens_;
        static size_t memAmount_;
        static size_t memWarnAmount_;
        static boost::mutex nbiomm_setup_mutex_;
};
    }
}

#endif
