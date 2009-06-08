#ifndef IOFWDUTIL_COMPLETION_IDALLOCATOR_HH
#define IOFWDUTIL_COMPLETION_IDALLOCATOR_HH

#include <boost/thread.hpp>
#include <vector>
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

/**
 * Small-object allocator that returns integer slots
 * T should be a POD type 
 *
 * @todo: consider switching to dequeue
 *
 * Does not do locking atm.
 */
template <typename T>
class IDAllocator 
{
public:
   IDAllocator (unsigned int reserve = 0) 
      : storage_(reserve), nextfree_(-1), size_(0)
   {
      STATIC_ASSERT(sizeof(MyType) >= sizeof(unsigned int)); 
   }

   ~IDAllocator ()
   {
      // Doesn't call destructors on cleanup; need to fix
      ALWAYS_ASSERT(size() == 0); 
   }

   unsigned int allocate (const T & n = T())
   {
      boost::mutex::scoped_lock l (lock_); 

      ++size_; 

      if (nextfree_ != -1)
      {
         // take element of free list
         unsigned int ret = nextfree_; 
         nextfree_ = getNextFree (ret); 
         do_construct (ret, n); 
         return ret; 
      }

      // no more free, need to extend
      unsigned int ret = static_cast<unsigned int>(storage_.size()); 
      storage_.push_back (MyType()); 
      do_construct (ret, n); 
      return ret; 
   }

   T & operator [] (unsigned int pos)
   { return reinterpret_cast<T &>(storage_[pos].value); } 
   
   const T & operator [] (unsigned int pos) const
   { return reinterpret_cast<const T &>(storage_[pos].value); } 


   inline void release (unsigned int id)
   {
      boost::mutex::scoped_lock l (lock_); 

      // Add element to nextfree list
      do_destruct (id); 
      setNextFree (id, nextfree_); 
      nextfree_ = id; 
      ASSERT(size_); 
      --size_; 
   }

   inline size_t size () const 
   { 
      // Just to be on the safe side
      boost::mutex::scoped_lock l (lock_); 
      return size_; 
   }

   inline size_t capacity () const
   { 
      boost::mutex::scoped_lock l (lock_); 
      return storage_.capacity (); 
   } 

protected:

   void do_construct (int pos, const T & n)
   {
      new(reinterpret_cast<T *>(&storage_[pos])) T (n);
   }

   void do_destruct (int pos)
   {
      (reinterpret_cast<T *>(&storage_[pos]))->~T (); 
   }

   unsigned int getNextFree (int ret)
   {
      return storage_[ret].nextfree; 
   }

   void setNextFree (int ret, unsigned int val)
   {
      storage_[ret].nextfree = val; 
   }

protected:

   typedef union 
   {
      unsigned int nextfree; 
      char         value[sizeof(T)]; 
   } MyType; 

   std::vector<MyType> storage_; 

   /// Next free element
   int nextfree_; 

   /// Number of entries in allocator
   unsigned int size_; 

   mutable boost::mutex lock_; 
}; 

//===========================================================================
   }
}

#endif
