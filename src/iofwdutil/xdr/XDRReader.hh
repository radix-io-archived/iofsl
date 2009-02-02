#ifndef IOFWDUTIL_XDRREADER_HH
#define IOFWDUTIL_XDRREADER_HH

#include "XDR.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

class XDRReader : public XDR
{
public:
  XDRReader (const char * mem, size_t memsize) :
      XDR (mem, memsize, true)
   {
   }

  template <typename T>
   XDRReader & operator >> (T & val)
   {
      *this  |=  (val); 
      return *this; 
   }

  template <typename T, int N>
  XDRReader & operator >> (T (&val)[N])
  {
     *this |=  (val); 
     return *this; 
  }

   
}; 


//===========================================================================
   }
}

#endif
