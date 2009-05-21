#include "IOFWDNotImplementedRequest.hh"
#include "zoidfs/zoidfs-wrapped.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"

using namespace iofwdutil::xdr; 

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
         beginReply (getXDRSize<int32_t>().max); 
         reply_writer_ << (int32_t) getReturnCode (); 
         sendReply (); 
      }

   }
}
