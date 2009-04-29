#ifndef IOFWDUTIL_XDR_XDRWRITER_HH
#define IOFWDUTIL_XDR_XDRWRITER_HH

#include "XDR.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

class XDRWriter : public XDR
{
public:
   enum { XDRTYPE = WRITER } ; 

   XDRWriter () 
      : XDR (0, 0, false)
   {
   }

   XDRWriter (void * mem, size_t memsize)
      : XDR (mem, memsize, false)
   {
   }

   const void * getBuf () const
   { 
      return mem_; 
   }

   size_t size () const
   {
      return getPos (); 
   }

   template <typename T>
   XDRWriter & operator << (const T & val)
   {
      process (*this, const_cast<T&>(val)); 
      //Processor<XDRWriter> w(*this);
      //w |= const_cast<T&>(val);
      return *this; 
   }

   template <typename T>
   XDRWriter & operator () (const T & val)
   {
      process (*this, const_cast<T&>(val)); 
      return *this; 
   }

};

//===========================================================================
   }
}

#endif
