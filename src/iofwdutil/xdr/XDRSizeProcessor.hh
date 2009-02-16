#ifndef IOFWDUTIL_XDR_XDRSIZEPROCESSOR_HH
#define IOFWDUTIL_XDR_XDRSIZEPROCESSOR_HH

// for size_t
#include <cstring>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include "iofwdutil/tools.hh"

// for uint8_t, ...
#include <inttypes.h>

#include "XDRWrappers.hh"
#include "XDRProcessor.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

/**
 * Class that calculates the size and max size of a XDR serialization
 */
class XDRSizeProcessor : public XDRProcessor
{
public:
   enum { XDRTYPE = SIZE } ; 

   XDRSizeProcessor () 
      : size_ (0), maxsize_ (0), onlyMax_(false)
   {
   }



   void reset ()
   {
      size_ = 0; maxsize_ = 0; 
   }

   template <typename T>
   XDRSizeProcessor & operator () (const T & t)
   { 
      process (*this, const_cast<T&>(t)); 
      return *this; 
   }

   template <typename T>
   XDRSizeProcessor & operator << (const T & t)
   {
      process (*this, const_cast<T&>(t)); 
      return *this; 
   }

public:
   class XDRSize
   {
   public:
      size_t actual;
      size_t max; 

      XDRSize (size_t a, size_t m)
         : actual(a), max(m)
      {
      }

   }; 

public:
   XDRSize getSize () const
   { return XDRSize (size_, maxsize_); }

public:
   size_t size_; 
   size_t maxsize_; 
   bool onlyMax_; 
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


template <typename T>
inline XDRSizeProcessor::XDRSize getXDRSize (const T & val)
{
   XDRSizeProcessor s; 
   process (s, const_cast<T&>(val)); 
   return s.getSize (); 
}


// FixedSizeOpaque always has same size 
inline void process (XDRSizeProcessor & f, const XDROpaque & o)
{
      f.size_ += o.size_; 
      f.maxsize_ += o.size_; 
}

inline size_t xdrsize_roundup4 (size_t s)
{
   return (((s+3)/4)*4);
}


inline void process (XDRSizeProcessor & f, const XDRString & s)
{
      f.size_ += xdrsize_roundup4(f.onlyMax_ ? s.maxsize_ : strlen (s.ptr_));
      f.maxsize_ += xdrsize_roundup4(s.maxsize_); 
      // length of string is added in front of string
      f.size_ += 4; 
      f.maxsize_ += 4; 
}

template <typename T, typename C>
void process (XDRSizeProcessor & f, const XDRVarArrayHelper<T,C> & a)
{
   // Array size: integer
   f.size_ += 4; 
   f.maxsize_ += 4; 

   for (unsigned int i=0; i<a.count_; ++i)
      process (f, a.ptr_[i]); 
   for (unsigned int i=0; i<a.maxcount_; ++i)
      process (f, a.ptr_[i]); 
}

template <typename T>
void process (XDRSizeProcessor & f, const XDREnumHelper<T> & UNUSED(e))
{
   f.size_ += 4; 
   f.maxsize_ += 4; 
}


#define XSP(type,size) \
   inline void process (XDRSizeProcessor & f, const type & UNUSED(v)) \
   { f.size_ += size; f.maxsize_+= size; }

// By default the XDR type is the size of the C type rounded up to 4 byte
// sizes
#define XSPDEFAULT(a) \
   XSP(a,(((sizeof(a)+3)/4)*4))
//#define XSPDEFAULT(a) 
//   XSP(a,4)

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
