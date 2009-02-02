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
   XDRWriter (char * mem, size_t memsize)
      : XDR (mem, memsize, false)
   {
   }

   template <typename T>
   XDRWriter & operator << (const T & val)
   {
      *this |= const_cast<T&>(val);
      return *this; 
   }

   template <typename T, int N>
   XDRWriter & operator << (const T (& val)[N])
   {
      *this |= const_cast<T(&)[N]>(val);
      return *this; 
   }


};

//===========================================================================
   }
}

#endif
