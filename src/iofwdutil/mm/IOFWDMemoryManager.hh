#ifndef IOFWD_IOFWD_MEMORY_MANAGER_HH
#define IOFWD_IOFWD_MEMORY_MANAGER_HH

#include <deque>
#include <boost/thread.hpp>
#include "iofwdutil/ConfigFile.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdutil/Singleton.hh"
#include "boost/thread/mutex.hpp"

namespace iofwdutil
{
    namespace mm
    {

/* forward decls */
class IOFWDMemoryManager;

/* wrapper for a single IOFWD memory allocation from a IOFWDMemoryManager*/
class IOFWDMemoryAlloc
{
    public:
        IOFWDMemoryAlloc()
        {
        } 

        virtual ~IOFWDMemoryAlloc()
        {
        }

        /* is the buffer allocated? */
        virtual bool allocated() const = 0;

        /* get the memory buffer start */
        virtual void * getMemory() const = 0;

        /* get the memory buffer start */
        virtual size_t getMemorySize() const = 0;

        /* get the number of tokens held by this alloc */
        virtual size_t getNumTokens() const = 0;

        virtual void alloc(int numTokens) = 0;
        virtual void dealloc() = 0;
};

/* allocation manager for IOFWD buffers */
class IOFWDMemoryManager
{
    public:
        IOFWDMemoryManager()
        {
        }

        virtual ~IOFWDMemoryManager()
        {
        }

        /* submit a buffer request */
        virtual void alloc(iofwdevent::CBType cb, IOFWDMemoryAlloc * memAlloc_) = 0;

        /* return a buffer to the manager */
        virtual void dealloc(IOFWDMemoryAlloc * memAlloc_) = 0;
};
    }
}

#endif
