#ifndef ZOIDFSUTIL_XDR_XDR_HH
#define ZOIDFSUTIL_XDR_XDR_HH

#include <rpc/xdr.h>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include "always_assert.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

class XDR
{
public:
   XDR (const char * mem, size_t size, bool read) :
      mem_(mem), size_(size), read_(read)
   {
      init (); 
   }

   void rewind ()
   {
      if (xdr_setpos (&xdr_, initpos_))
         return;
      // If setpos failed, recreate XDR mem 
      destroy (); 
      init (); 
   }
   
   ~XDR ()
   {
      destroy (); 
   }

protected:
   struct ::XDR xdr_; 
   
   void check (int t)
   {
      ALWAYS_ASSERT (t); 
   }

protected:
   void destroy ()
   {
      xdr_destroy (&xdr_); 
   }

   void init ()
   {
      xdrmem_create (&xdr_, const_cast<char*>(mem_),
            size_, (read_ ? XDR_DECODE : XDR_ENCODE)); 
      initpos_ = xdr_getpos(&xdr_);
   }

public:
   // Special functions
   XDR & processFixedSizeOpaque (void * data, size_t bytes)
   {
      check(xdr_opaque(&xdr_, (char*) data, bytes)); 
      return *this; 
   }

   XDR & processString (char * data, size_t maxbytes)
   {
      check(xdr_string(&xdr_, &data, maxbytes)); 
      return *this; 
   }

   template <typename T>
   XDR & processEnum (T & e)
   {
      BOOST_STATIC_ASSERT (sizeof (T) <= sizeof (enum_t));
      // might be faster here to skip the read/write test all together
      enum_t fixed;
      if (!read_)
         fixed = e; 
      check(xdr_enum(&xdr_, &fixed));
      if (read_)
         e = static_cast<T>(fixed); 
      return *this; 
   }

protected:
   template <typename T>
   friend XDR & operator |= (XDR & x, T & s); 

   bool read_; 
   const char * mem_;
   size_t size_; 
   unsigned int initpos_; 
}; 
//===========================================================================

  // Disabled to make sure we explicitly use encodeFixedSizeOpaque 
  // or encodeFixedSizeString, ...
  /* template <typename T, int N>
   XDR &  operator |= (XDR & x, T (&s)[N])
   {
      /// Could use xdr_vector here but since xdr_vector just calls the
      /// function pointer to the individual xdr_ ... function we call the
      /// function here directly.
      for (unsigned int i=0; i<N; ++i)
         x |= s[i]; 
      return x; 
   } */

   
   /// Fallback method to prevent implicit type conversion 
   /// After all, we want to make sure we know *exactly* which
   /// type gets encoded/decoded onto/from the XDR stream
   template <typename T>
   XDR & operator |= (XDR & x, T & s)
   {
      // Force compiler to postpone evaluation until 
      // instantiated.
      BOOST_STATIC_ASSERT (sizeof(T) == 0) ; 
      return x; 
   }


#define XDRHELPER(type,func) \
   template <> \
   inline XDR &  operator |=<type> (XDR & x, type & s) \
   { x.check(xdr_##func (&x.xdr_, &s)); return x;  }

#define XDRHELPER2(type) XDRHELPER(type,type)

/*template <>
inline XDR & operator |=<signed char> (XDR & x, signed char & s)
{
   x.check(xdr_char (&x.xdr_, (char*) &s)); return x; 
}*/

// Will need fixes here on 32 bit to determine if we need
// long long (or uint64_t/int64_t)

//XDRHELPER2(char); 
//XDRHELPER2(short); 
//XDRHELPER2(int);
//XDRHELPER2(long); 

//XDRHELPER2(double); 
//XDRHELPER2(float); 

//XDRHELPER(unsigned char, u_char);
//XDRHELPER(unsigned short, u_short);
//XDRHELPER(unsigned int, u_int);
//XDRHELPER(unsigned long, u_long);


XDRHELPER2(char); 

XDRHELPER2(uint8_t);
XDRHELPER2(uint16_t);
XDRHELPER2(uint32_t);
XDRHELPER2(uint64_t);

XDRHELPER2(int8_t);
XDRHELPER2(int16_t);
XDRHELPER2(int32_t);
XDRHELPER2(int64_t);


// bool_t is defined as int or some other integral type
// XDRHELPER(bool_t,bool); 

#undef XDRHELPER

//===========================================================================
   }
}

#endif
