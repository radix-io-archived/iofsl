#ifndef IOFWDUTIL_XDR_XDRWRAPPERS_HH
#define IOFWDUTIL_XDR_XDRWRAPPERS_HH

#include <boost/assert.hpp>

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

// Some wrapper types for cases where we want to make sure the C++ compiler
// doesn't automatically cast to something we don't want or when we need more
// information

class XDRString 
{
public:
   XDRString (const char * ptr, size_t maxsize)
      : ptr_(const_cast<char*>(ptr)), maxsize_(maxsize)
   {
      BOOST_ASSERT (ptr); 
   }

   mutable char * ptr_; 
   mutable size_t maxsize_; 
}; 

class XDROpaque 
{
public:
   XDROpaque (const void * ptr, size_t size)
      : ptr_(const_cast<void*>(ptr)), size_(size)
   {
      BOOST_ASSERT(ptr); 
   }

   void * ptr_; 
   size_t size_; 
};

template <typename T>
class XDREnumHelper 
{
public:
   XDREnumHelper (T & en)
      : enum_ (en)
   {
   }

   T & enum_; 
};

template <typename T>
XDREnumHelper<T> XDREnum (T & en)
{
   return XDREnumHelper<T> (en); 
}


/**
 * Note: although the exact type of the count and maxcount variable is
 * a template for making usage of the class easier, 
 * the XDR standard requires that the size parameter is serialized 
 * as a 32 bit unsigned integer (which is what we do).
 */
template <typename T, typename C> 
class XDRVarArrayHelper
{
public:
   XDRVarArrayHelper (T * ptr, C & count)
      : ptr_(ptr), count_(count), maxcount_(count)
   {
   }

   XDRVarArrayHelper (T * ptr, C & count, C maxcount)
      : ptr_(ptr), count_(count), maxcount_(maxcount)
   {
   }
   T * ptr_; 
   C & count_; 
   C maxcount_; 
}; 

// XDR array with count elements and space for count elements
template <typename T, typename C>
XDRVarArrayHelper<T,C> XDRVarArray (T * arrayptr, C & count)
{
   return XDRVarArrayHelper<T,C> (arrayptr, count, count); 
}

// XDR array with count elements and room for maxcount elements
template <typename T, typename C>
XDRVarArrayHelper<T,C> XDRVarArray (T * arrayptr, C & count, C maxcount)
{
   return XDRVarArrayHelper<T,C> (arrayptr, count, maxcount); 
}
//===========================================================================
   }
}

#endif
