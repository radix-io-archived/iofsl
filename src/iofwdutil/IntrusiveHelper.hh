#ifndef IOFWDUTIL_INTRUSIVE_HELPER_HH
#define IOFWDUTIL_INTRUSIVE_HELPER_HH

#include "iofwdutil/assert.hh"
#include "iofwdutil/atomics.hh"

#include <boost/utility.hpp>

namespace iofwdutil
{
   //========================================================================

   /**
    * Helper class to use with boost intrusive_ptr
    *
    * Usage: derive your class from this class,
    * and add the macro INTRUSIVE_PTR_HELPER(yourclassname);
    * in the class header.
    *
    * Now you can use your class with the boost intrusive shared pointer.
    *
    * !!! DO NOT DERIVE FROM CLASSES THAT DERIVE FROM INTRUSIVEHELPER !!!
    * (unless the base class has a virtual destructor)
    *
    * Note: the reference count update functions are const since we need to be
    * able to support intrusive_ptr<const T>
    */
   class IntrusiveHelper
   {
      public:
         IntrusiveHelper ()
            : refcount_ (0)
         {
         }

         ~IntrusiveHelper ()
         {
            /* Make sure nobody is still pointing to us */
            ASSERT(!refcount_);
         }

         int removeref () const
         {
            ASSERT(refcount_);
            return --refcount_;
         }

         int addref () const
         {
            return ++refcount_;
         }

         /**
          * Returns true if there are still shared pointers pointing to us.
          */
         int alive () const
         {
            return refcount_;
         }

      private:
         mutable atomic<int> refcount_;
   };


   /* @NOTE:
    *    * Can simplify this by providing a template version
    *      of the functions below.
    *    * The code below is threadsafe since the reference count is
    *      stored in an atomic<int> var.
    */
#define INTRUSIVE_PTR_HELPER(cls) \
   inline void intrusive_ptr_add_ref(const cls * r) \
   { r->addref(); } \
   \
   inline void intrusive_ptr_release(const cls * r)\
   { if (!r->removeref()) { boost::checked_delete (r); } }

   //========================================================================
}

#endif
