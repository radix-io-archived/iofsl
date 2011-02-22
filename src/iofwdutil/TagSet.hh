#ifndef IOFWDUTIL_TAGSET_HH
#define IOFWDUTIL_TAGSET_HH

#include <limits>
#include <deque>
#include <algorithm>

#include <boost/thread.hpp>

namespace iofwdutil
{
   //========================================================================

   /**
    * Provide a (local) unique tag.
    * If this proves to a be a serialization/contention point,
    * consider doing TLS for the tags.
    *
    * Class is thread safe
    */
   template <typename BASE>
   class TagSet
   {
      public:
         TagSet (BASE start = std::numeric_limits<BASE>::min(),
               BASE stop = std::numeric_limits<BASE>::max())
            : start_(start), stop_(stop), first_free_(start)
         {
         }

         BASE getTag ();

         void releaseTag (BASE tag);

      protected:
         boost::mutex lock_;

         BASE start_;
         BASE stop_;
         BASE first_free_;

         std::deque<BASE> free_;
   };

   //========================================================================
   
   template <typename BASE>
   BASE TagSet<BASE>::getTag ()
   {
      boost::mutex::scoped_lock l(lock_);
      if (free_.size())
      {
         BASE b (free_.back());
         free_.pop_back();
         return b;
      }

      BASE ret = first_free_++;
      BASE delta = std::min(static_cast<size_t>(stop_ - first_free_),
            static_cast<size_t>(free_.size() ?  free_.size() : (size_t) 1));
      ALWAYS_ASSERT(delta);
      for (BASE i=1; i<delta; ++i)
      {
         free_.push_back (first_free_++);
      }
      return ret;
   }

   template <typename BASE>
   void TagSet<BASE>::releaseTag (BASE b)
   {
      boost::mutex::scoped_lock l(lock_);
      free_.push_back (b);
   }

   //========================================================================
}

#endif
