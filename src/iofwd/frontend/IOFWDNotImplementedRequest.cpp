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

      void IOFWDNotImplementedRequest::reply (const CBType & cb)
      {
         simpleReply (cb, TSSTART << (int32_t) getReturnCode());
      }

//===========================================================================
   }
}
