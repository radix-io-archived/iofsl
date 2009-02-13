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
   {
   }

   XDRWriter (char * mem, size_t memsize)
      : XDR (mem, memsize, false)
   {
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
