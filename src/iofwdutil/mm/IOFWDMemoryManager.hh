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

class GenericMemoryManager : 
   public IOFWDMemoryAlloc
{
   public:
      GenericMemoryManager ()
      {
         bufferSize_ =  4 * 1024 * 1024;
      }
      void alloc (int numTokens)
      {
          allocated_ = true;
          numTokens_ = numTokens;
          /* create the memory buffer */
          memory_ = (void *) new char[numTokens];     
      }
      /*
       * This is the public buffer allocation method
       */
      void alloc(int numTokens, size_t bufferSize)
      {
          /* set the memory params */
          allocated_ = true;
          numTokens_ = numTokens;
          bufferSize_ = bufferSize;

          /* create the memory buffer */
          memory_ = (void *) new char[bufferSize_];          
      }

      void dealloc() 
      {
         if (allocated_)
            delete[] (char*)memory_;
      }
      ~GenericMemoryManager()
      {
          /* if we allocated a buffer, dealloc it */
          if(allocated_)
              dealloc();
      }

      bool allocated() const
      {
          return allocated_;
      }

      void * getMemory() const
      {
          return memory_;
      }

      size_t getMemorySize() const
      {
          return bufferSize_;
      }

      size_t getReqMemorySize() const
      {
          return bufferSize_;
      }

      size_t getNumTokens() const
      {
          return numTokens_;
      }

   protected:
      /* allocation valid flag */
      bool allocated_;

      /* number of tokens owned by this alloc */
      size_t numTokens_;
      size_t bufferSize_;

      /* the memory buffer */
      void * memory_;

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
