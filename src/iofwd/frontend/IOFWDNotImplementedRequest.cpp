#include "IOFWDNotImplementedRequest.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
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
        
         simpleReply (TSSTART << (int32_t) getReturnCode()); 

         /*beginReply (getXDRSize<int32_t>().max); 
         reply_writer_ << (int32_t) getReturnCode (); 
         sendReply (); */
      }

   }
}
