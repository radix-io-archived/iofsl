#ifndef IOFWDUTIL_INTRUSIVE_HELPER_HH
#define IOFWDUTIL_INTRUSIVE_HELPER_HH

#include "iofwdutil/assert.hh"

namespace iofwdutil
{

/**
 * Helper class to use with boost intrusive_ptr
 *
 * Usage: derive your class from this class,
 * and add the macro INTRUSIVE_PTR_HELPER(yourclassname);
 * in the class header.
 *
 * Now you can use your class with the boost intrusive shared pointer.
 */
class IntrusiveHelper  
{
public:
   IntrusiveHelper () : 
      refcount_ (0)
   {
   }

   ~IntrusiveHelper ()
   {
      /* Make sure nobody is still pointing to us */
      ASSERT(!refcount_);
   }

   int removeref ()
   {
      ASSERT(refcount_); 
      return --refcount_; 
   }

   int addref ()
   {
      return ++refcount_; 
   }

   /**
    * Returns true if there are still shared pointers pointing to us.
    */
   bool alive () const
   {
      return refcount_;
   }
private:
   int refcount_; 
}; 


/* @NOTE:
 *    * Can simplify this by providing a template version
 *      of the functions below.
 *    * This is not thread safe; Can make it thread safe 
 *      for all versions or just for certain classes 
 *      by providing another PTR_HELPER macro or specializing
 *      the template function.
 */
#define INTRUSIVE_PTR_HELPER(cls) \
   inline void intrusive_ptr_add_ref(cls * r) \
   { r->addref(); } \
   \
   inline void intrusive_ptr_release(cls * r)\
   { if (!r->removeref()) { delete r; } }



}

#endif
