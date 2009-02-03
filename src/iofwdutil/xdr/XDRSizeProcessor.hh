#ifndef IOFWDUTIL_XDR_XDRSIZEPROCESSOR_HH
#define IOFWDUTIL_XDR_XDRSIZEPROCESSOR_HH

// for size_t
#include <cstring>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>

// for uint8_t, ...
#include <inttypes.h>


namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

/**
 * Class that calculates the size and max size of a XDR serialization
 */
class XDRSizeProcessor
{
public:
   XDRSizeProcessor () 
      : size_ (0), maxsize_ (0)
   {
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

   // FixedSizeOpaque always has same size 
   void processFixedSizeOpaque (XDRSizeProcessor & f, void * ptr, size_t size)
   {
      f.size_ += size; 
      f.maxsize_ += size; 
   }

   void processString (XDRSizeProcessor & f, char * data, size_t maxbytes)
   {
      f.size_ += (f.onlyMax_ ? maxbytes : strlen (data));
      f.maxsize_ += maxbytes; 
      // length of string is added in front of string
      f.size_ += 4; 
      f.maxsize_ += 4; 
   }

   template <typename T>
   void processEnum (XDRSizeProcessor & f, T & e)
   {
      f.size_ += 4; 
      f.maxsize_ += 4; 
   }

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

#define XSP(type,size) \
   inline void process (XDRSizeProcessor & f, const type & v) \
   { f.size_ += size; f.maxsize_+= size; }

// By default the XDR type is the size of the C type rounded up to 4 byte
// sizes
#define XSPDEFAULT(a) \
   XSP(a,(((sizeof(a)+3)/4)*4))

//XSPDEFAULT(unsigned char);
XSPDEFAULT(uint8_t);
XSPDEFAULT(uint16_t);
XSPDEFAULT(uint32_t);
XSPDEFAULT(uint64_t);
XSPDEFAULT(int8_t);
XSPDEFAULT(int16_t);
XSPDEFAULT(int32_t);
XSPDEFAULT(int64_t);

#undef XSP

//===========================================================================
   }
}

#endif
