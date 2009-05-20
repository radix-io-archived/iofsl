#include "IOFWDNotImplementedRequest.hh"
#include "zoidfs/zoidfs-wrapped.hh"

namespace iofwd
{
   namespace frontend
   {

      IOFWDNotImplementedRequest::~IOFWDNotImplementedRequest ()
      {
      }

      void IOFWDNotImplementedRequest::reply ()
      {
         setReturnCode (zoidfs::ZFSERR_NOTIMPL); 
      }

   }
}
