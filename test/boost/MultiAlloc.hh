#ifndef TEST_BOOST_MULTIALLOC_HH
#define TEST_BOOST_MULTIALLOC_HH

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <boost/foreach.hpp>

/**
 * Allocates a set of buffers
 */
class MultiAlloc
{
   public:
      MultiAlloc ()
      {
      }

      MultiAlloc (size_t maxbufcount, size_t maxbufsize)
      {
         size_t count = random () % maxbufcount;
         mem_.reserve (count);
         memsizes_.reserve (count);
         for (size_t i=0; i<count; ++i)
         {
            memsizes_.push_back (random () % maxbufsize);
            mem_.push_back (new char[memsizes_.back()]);
         }
      }

      /**
       * Returns true if no buffers are allocated.
       */
      bool empty () const
      {
         return !size(); 
      }

      /**
       * Return number of buffers in set
       */
      size_t size () const
      {
         return memsizes_.size();
      }

      /**
       * Return total number of bytes in allocation
       */
      size_t totalSize () const
      {
         return std::accumulate (memsizes_.begin(), memsizes_.end(), 0);
      }

      /**
       * Return array of buffer sizes
       */
      const size_t * getMemSizes () const
      { return (size() ? &memsizes_[0] : 0); }

      size_t * getMemSizes ()
      { return (size() ? &memsizes_[0] : 0); }

      /// Return array of buffer pointers
      void ** getMem ()
      { return (size() ? &mem_[0] : 0); }

      void * const * getMem () const
      { return (size() ? &mem_[0] : 0); }

      /// Add buffer at the end. Return new buffer
      void * addBuf (size_t bufsize)
      {
         memsizes_.push_back (bufsize);
         mem_.push_back (new char [bufsize]);
         return mem_.back ();
      }

      /// Add buffer of random size. Return new buffer & size
      std::pair<void *, size_t> addRandomBuf (size_t maxsize)
      {
         const size_t s = random () % maxsize;
         return std::make_pair (addBuf (s), s);
      }

      ~MultiAlloc ()
      {
         reset ();
      }

      /**
       * Free all buffers
       */
      void reset ()
      {
         BOOST_FOREACH (void * ptr, mem_)
         {
            delete[] (static_cast<char*>(ptr));
         }
         mem_.clear (); memsizes_.clear();
      }

      void swap (MultiAlloc & other)
      {
         mem_.swap (other.mem_);
         memsizes_.swap (other.memsizes_);
      }

   protected:
      std::vector<void *> mem_;
      std::vector<size_t> memsizes_;
};

#endif
