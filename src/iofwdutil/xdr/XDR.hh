#ifndef ZOIDFSUTIL_XDR_XDR_HH
#define ZOIDFSUTIL_XDR_XDR_HH

#include <rpc/xdr.h>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include "always_assert.hh"
#include "XDRWrappers.hh"
#include "XDRProcessor.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

// Processor for XDR  
class XDR  : public XDRProcessor 
{
public:
   XDR (const void * mem, size_t size, bool read) :
      mem_(mem), size_(size), read_(read)
   {
      init (); 
   }

   XDR () 
      : mem_(0), size_(0), read_(true)
   {
      init (); 
   }

   void reset (const void * mem, size_t size)
   {
      destroy (); 
      mem_ = static_cast<const void*>(mem); size_ = size; 
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

   size_t getPos () const 
   {
      return xdr_getpos (&xdr_); 

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

      xdrmem_create (&xdr_, const_cast<char *> (static_cast<const char *> (mem_)),
            (unsigned int) size_, (read_ ? XDR_DECODE : XDR_ENCODE)); 
      initpos_ = xdr_getpos(&xdr_);
   }

public:
   const void * mem_;
   size_t size_; 
   bool read_; 
   unsigned int initpos_; 
}; 
//===========================================================================
#define XDRHELPER(type,func) \
   inline void process (XDR & f, type & s) \
   { f.check(xdr_##func (&f.xdr_, &s));  }

#define XDRHELPER2(type) XDRHELPER(type,type)

XDRHELPER2(char)

XDRHELPER2(uint8_t)
XDRHELPER2(uint16_t)
XDRHELPER2(uint32_t)
XDRHELPER2(uint64_t)

XDRHELPER2(int8_t)
XDRHELPER2(int16_t)
XDRHELPER2(int32_t)
XDRHELPER2(int64_t)


// bool_t is defined as int or some other integral type
// XDRHELPER(bool_t,bool); 

#undef XDRHELPER


// Processor functions 
inline void process (XDR & f, const XDROpaque & o)
{
   f.check(xdr_opaque(&f.xdr_, (char*) o.ptr_, (unsigned int) o.size_)); 
}

inline void process (XDR & f, const XDRString & s)
{
   f.check(xdr_string(&f.xdr_, &s.ptr_, (unsigned int) s.maxsize_)); 
}

template <typename T, typename C> 
inline void process (XDR & f, const XDRVarArrayHelper<T,C> & a)
{
   // send/receive array count
   uint32_t count = static_cast<uint32_t>(a.count_); 
   process (f, count); 
   a.count_ = count; 

   ALWAYS_ASSERT (a.count_ <= a.maxcount_); 

   // send/receive array elements 
   for (unsigned int i=0; i<a.count_; ++i)
      process (f, a.ptr_[i]); 
}

template <typename T>
inline void process (XDR & f, const XDREnumHelper<T> & e)
{
   BOOST_STATIC_ASSERT (sizeof (T) <= sizeof (enum_t));
   // might be faster here to skip the read/write test all together
   enum_t fixed;
   if (!f.read_)
      fixed = e.enum_; 
   f.check(xdr_enum(&f.xdr_, &fixed));
   if (f.read_)
      e.enum_ = static_cast<T>(fixed); 
}


//===========================================================================
   }
}

#endif
