#ifndef ENCODER_XDR_XDRSIZEPROCESSOR_HH
#define ENCODER_XDR_XDRSIZEPROCESSOR_HH

// for size_t
#include <cstring>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include "iofwdutil/tools.hh"

// for uint8_t, ...
#include <inttypes.h>

#include "encoder/Size.hh"
#include "encoder/EncoderWrappers.hh"

namespace encoder
{
   namespace xdr
   {
//===========================================================================

/**
 * Class that calculates the size and max size of a XDR serialization
 */
class XDRSizeProcessor : public Size
{
public:

   template <typename T>
   XDRSizeProcessor & operator () (T & t)
   {
      process (*this, t);
      return *this;
   }

   template <typename T>
   XDRSizeProcessor & operator << (T & t)
   {
      process (*this, t);
      return *this;
   }
};

   /*template <typename T>
   void process (XDRSizeProcessor & f, const T & val)
   {
      process (f, const_cast<T&>(val));
   }*/

   /*template <typename T>
   void process (XDRSizeProcessor & f, T & val)
    {
       BOOST_STATIC_ASSERT (sizeof(T) <= 0);
   } */


/**
 * Helper function:
 *   return size of T in XDR environment.
 */
template <typename T>
inline Size::SizeInfo getXDRSize ()
{
   const T dummy = T();
   XDRSizeProcessor s;
   process (s, dummy);
   return s.size();
}

template <typename T>
inline Size::SizeInfo getXDRSize (const T & dummy)
{
   XDRSizeProcessor s;
   process (s, dummy);
   return s.size();
}


// FixedSizeOpaque always has same size
inline void process (XDRSizeProcessor & f, const EncOpaque & o)
{
      f.incActualSize (o.size_);
      f.incMaxSize (o.size_);
}

inline size_t xdrsize_roundup4 (size_t s)
{
   return (((s+3)/4)*4);
}


inline void process (XDRSizeProcessor & f, const EncString & s)
{
   // Only calculate actual string len when the user wants to know it.
   // Otherwise, use max string len.
   f.incActualSize (xdrsize_roundup4(!f.isActualValid() ? s.maxsize_ :
         (s.ptr_ ?  strlen (s.ptr_) : 0 )));
   f.incMaxSize (xdrsize_roundup4(s.maxsize_));
   // length of string is added in front of string
   f.incActualSize (4);
   f.incMaxSize (4);
}

template <typename T, typename C>
void process (XDRSizeProcessor & f, const EncVarArrayHelper<T,C> & a)
{
   // Array size: integer
   f.incActualSize (4);
   f.incMaxSize (4);

   for (unsigned int i=0; i<a.count_; ++i)
      process (f, a.ptr_[i]);
   for (unsigned int i=0; i<a.maxcount_; ++i)
      process (f, a.ptr_[i]);
}

template <typename T>
void process (XDRSizeProcessor & f, const EncEnumHelper<T> & UNUSED(e))
{
   f.incActualSize (4);
   f.incMaxSize (4);
}


#define XSP(type,size) \
   inline void process (XDRSizeProcessor & f, const type & UNUSED(v)) \
   { f.incActualSize (size); f.incMaxSize (size); }

// By default the XDR type is the size of the C type rounded up to 4 byte
// sizes
#define XSPDEFAULT(a) \
   XSP(a,(((sizeof(a)+3)/4)*4))

//XSPDEFAULT(unsigned char);
XSPDEFAULT(uint8_t)
XSPDEFAULT(uint16_t)
XSPDEFAULT(uint32_t)
XSPDEFAULT(uint64_t)
XSPDEFAULT(int8_t)
XSPDEFAULT(int16_t)
XSPDEFAULT(int32_t)
XSPDEFAULT(int64_t)

#undef XSP

//===========================================================================
   }
}

#endif
