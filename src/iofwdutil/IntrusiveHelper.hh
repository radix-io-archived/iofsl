#ifndef IOFWDUTIL_INTRUSIVE_HELPER_HH
#define IOFWDUTIL_INTRUSIVE_HELPER_HH

#include "iofwdutil/assert.hh"

namespace iofwdutil
{

/**
 * Helper class to use with boost intrusive_ptr
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
   }

   int unref ()
   {
      ASSERT(refcount_); 
      return --refcount_; 
   }

   int ref ()
   {
      return ++refcount_; 
   }

private:
   int refcount_; 
}; 

#define INTRUSIVE_PTR_HELPER(cls) \
   inline void intrusive_ptr_add_ref(cls * r) \
   { r->ref(); } \
   \
   inline void intrusive_ptr_release(cls * r)\
   { if (!r->unref()) { delete r; } }



}

#endif
