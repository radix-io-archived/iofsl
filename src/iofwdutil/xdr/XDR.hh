#ifndef ZOIDFSUTIL_XDR_XDR_HH
#define ZOIDFSUTIL_XDR_XDR_HH

#include <rpc/xdr.h>
#include <boost/static_assert.hpp>

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
   
   /// Fallback method to prevent implicit type conversion 
   /// After all, we want to make sure we know *exactly* which
   /// type gets encoded/decoded onto/from the XDR stream
   template <typename T>
   void transfer (T & s)
   {
      // Force compiler to postpone evaluation until 
      // instantiated.
      BOOST_STATIC_ASSERT (sizeof(T) == 0) ; 
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

   void check (int t)
   {
   }

protected:
   struct ::XDR xdr_; 
   bool read_; 
   const char * mem_;
   size_t size_; 
   unsigned int initpos_; 
}; 
//===========================================================================


#define XDRHELPER(type,func) \
   template <> \
   inline void XDR::transfer<type> (type & s) \
   { check(xdr_##func (&xdr_, &s)); }

#define XDRHELPER2(type) XDRHELPER(type,type)

XDRHELPER2(int);
XDRHELPER2(char); 
XDRHELPER2(long); 
XDRHELPER2(double); 
XDRHELPER2(float); 
XDRHELPER2(short); 

XDRHELPER(unsigned char, u_char);
XDRHELPER(unsigned int, u_int);
XDRHELPER(unsigned long, u_long);
XDRHELPER(unsigned short, u_short);



// bool_t is defined as int or some other integral type
// XDRHELPER(bool_t,bool); 

#undef XDRHELPER

//===========================================================================
   }
}

#endif
