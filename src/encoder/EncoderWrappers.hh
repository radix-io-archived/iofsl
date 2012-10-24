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
 * Encoder wrapper for a typical C string (variable length).
 * Maxsize indicates the maximum number of CHARACTERS in the string, not the
 * maximum encoded size. (i.e. 0 terminators or anything like that is not
 * counted).
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
 * ptr shoud point to a buffer of at least size bytes.
 */
class EncOpaque
{
public:
   EncOpaque (const void * ptr, size_t size, size_t maxsize = 0)
      : ptr_(const_cast<void*>(ptr)), size_(size)
   {
      if (maxsize == 0)
         maxsize_ = size;
      else 
         maxsize_ = maxsize;
   }

   mutable void * ptr_;
   const size_t size_;
   size_t maxsize_;
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

   T & enum_;
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
 * It should probably be restricted to basic types for T, to avoid calculating
 * maxsize problems. Or at least to compile time sized objects. Think about it.
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
