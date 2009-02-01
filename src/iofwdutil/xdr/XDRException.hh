#ifndef IOFWDUTIL_XDR_XDREXCEPTION_HH
#define IOFWDUTIL_XDR_XDREXCEPTION_HH

#include "ZException.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================

      class XDRException : public ZException
      {
         ~XDRException (); 
      }; 


//===========================================================================
   }
}

#endif
