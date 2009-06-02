#ifndef IOFWDUTIL_COMPLETION_IDALLOCATOR_HH
#define IOFWDUTIL_COMPLETION_IDALLOCATOR_HH

#include "iofwdutil/assert.hh"
#include <vector>

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
   }

   unsigned int allocate ()
   {
      ++size_; 

      if (nextfree_ != -1)
      {
         // take element of free list
         unsigned int ret = nextfree_; 
         nextfree_ = storage_[ret].nextfree; 
         return ret; 
      }

      // no more free, need to extend
      unsigned int ret = storage_.size(); 
      storage_.push_back (MyType()); 
      return ret; 
   }

   T & operator [] (unsigned int pos)
   { return storage_[pos].value; } 

   inline void release (unsigned int id)
   {
      // Add element to nextfree list
      storage_[id].nextfree = nextfree_; 
      nextfree_ = id; 
      ASSERT(size); 
      --size_; 
   }

   inline size_t size () const 
   { return size_; }

   inline size_t capacity () const
   { return storage_.capacity (); } 


protected:

   typedef union 
   {
      int nextfree; 
      T   value; 
   } MyType; 

   std::vector<MyType> storage_; 

   /// Next free element
   int nextfree_; 

   /// Number of entries in allocator
   unsigned int size_; 
}; 

//===========================================================================
   }
}

#endif
