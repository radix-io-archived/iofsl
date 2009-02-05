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

// Processor for XDR  
class XDR 
{
public:
   XDR (const char * mem, size_t size, bool read) :
      mem_(mem), size_(size), read_(read)
   {
      init (); 
   }

   XDR () 
      : mem_(0), size_(0), read_(true)
   {
      init (); 
   }

   void reset (const char * mem, size_t size, bool read)
   {
      destroy (); 
      mem_ = mem; size_ = size; read_ = read; 
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

public:
   struct ::XDR xdr_; 
   
   void check (int t)
   {
      ALWAYS_ASSERT (t); 
   }

protected:
   void destroy ()
   {
      if (mem_)
         xdr_destroy (&xdr_); 
      mem_ = 0; 
      size_ = 0; 
   }

   void init ()
   {
      if (!mem_)
         return ; 

      xdrmem_create (&xdr_, const_cast<char*>(mem_),
            size_, (read_ ? XDR_DECODE : XDR_ENCODE)); 
      initpos_ = xdr_getpos(&xdr_);
   }

public:
   bool read_; 
   const char * mem_;
   size_t size_; 
   unsigned int initpos_; 
}; 
//===========================================================================

// Processor functions 
   void processFixedSizeOpaque (XDR & f, void * data, size_t bytes)
   {
      f.check(xdr_opaque(&f.xdr_, (char*) data, bytes)); 
   }

   void processString (XDR & f, char * data, size_t maxbytes)
   {
      f.check(xdr_string(&f.xdr_, &data, maxbytes)); 
   }

   template <typename T>
   void processEnum (XDR & f, T & e)
   {
      BOOST_STATIC_ASSERT (sizeof (T) <= sizeof (enum_t));
      // might be faster here to skip the read/write test all together
      enum_t fixed;
      if (!f.read_)
         fixed = e; 
      f.check(xdr_enum(&f.xdr_, &fixed));
      if (f.read_)
         e = static_cast<T>(fixed); 
   }

  /* template <typename T>
   void process (XDR & f, T & val)
   {
      // make instantiating this method fail
      BOOST_STATIC_ASSERT(sizeof(T)==0); 
   }*/


#define XDRHELPER(type,func) \
   inline void process (XDR & f, type & s) \
   { f.check(xdr_##func (&f.xdr_, &s));  }

#define XDRHELPER2(type) XDRHELPER(type,type)

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
