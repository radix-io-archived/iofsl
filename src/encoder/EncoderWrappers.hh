#ifndef ENCODER_ENCODERWRAPPERS_HH
#define ENCODER_ENCODERWRAPPERS_HH

#include <boost/assert.hpp>

namespace encoder
{
//===========================================================================

// Some wrapper types for cases where we want to make sure the C++ compiler
// doesn't automatically cast to something we don't want or when we need to
// pass in more information (for example maximum buffer size)


/**
 * Encoder wrapper for a typical C string.
 */
class EncString
{
public:
   EncString (const char * ptr, size_t maxsize)
      : ptr_(const_cast<char*>(ptr)), maxsize_(maxsize)
   {
   }

   mutable char * ptr_;
   mutable size_t maxsize_;
};


/**
 * Opaque fixed-length binary data.
 */
class EncOpaque
{
public:
   EncOpaque (const void * ptr, size_t size)
      : ptr_(const_cast<void*>(ptr)), size_(size)
   {
   }

   mutable void * ptr_;
   size_t size_;
};


/**
 * Wrapper for a specific kind of enum
 */
template <typename T>
class EncEnumHelper
{
public:
   EncEnumHelper (T & en)
      : enum_ (en)
   {
   }

   mutable T & enum_;
};

/**
 * Helper function for enums so that we don't have to specify the class
 * template type.
 */
template <typename T>
EncEnumHelper<T> EncEnum (T & en)
{
   return EncEnumHelper<T> (en);
}


/**
 * Variable size array.
 */
template <typename T, typename C>
class EncVarArrayHelper
{
public:
   EncVarArrayHelper (T * ptr, C & count)
      : ptr_(ptr), count_(count), maxcount_(count)
   {
   }

   EncVarArrayHelper (T * ptr, C & count, C maxcount)
      : ptr_(ptr), count_(count), maxcount_(maxcount)
   {
   }
   T * ptr_;
   C & count_;
   C maxcount_;
};

// Enc array with count elements and space for count elements
template <typename T, typename C>
EncVarArrayHelper<T,C> EncVarArray (T * arrayptr, C & count)
{
   return EncVarArrayHelper<T,C> (arrayptr, count, count);
}

// XDR array with count elements and room for maxcount elements
template <typename T, typename C>
EncVarArrayHelper<T,C> EncVarArray (T * arrayptr, C & count, C maxcount)
{
   return EncVarArrayHelper<T,C> (arrayptr, count, maxcount);
}
//===========================================================================
}

#endif
