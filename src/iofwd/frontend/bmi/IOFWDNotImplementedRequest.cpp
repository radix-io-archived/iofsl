#include "IOFWDNotImplementedRequest.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"

using namespace encoder::xdr;

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

      IOFWDNotImplementedRequest::~IOFWDNotImplementedRequest ()
      {
      }

      // @TODO: most of the reply function are the same; One implementation in
      // IOFWDRequest would suffice and only some special requests
      // (read/readlink/readdir) would need to override them
      void IOFWDNotImplementedRequest::reply (const CBType & cb)
      {
         simpleReply (cb, TSSTART << (int32_t) getReturnCode());
      }

//===========================================================================
   }
}
