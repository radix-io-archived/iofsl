#ifndef IOFWD_NOTIMPLEMENTEDREQUEST_HH
#define IOFWD_NOTIMPLEMENTEDREQUEST_HH

#include "Request.hh"

namespace iofwd
{
//===========================================================================

class NotImplementedRequest : public Request
{
public:
   NotImplementedRequest (int opid)
      : Request (opid)
   {
      // NotImplementedRequest always fails with the same error code.
      setReturnCode (zoidfs::ZFSERR_NOTIMPL);
   }

   virtual void reply (const CBType & cb) = 0;

};


//===========================================================================
}

#endif
