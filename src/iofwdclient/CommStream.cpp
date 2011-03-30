#include "iofwdclient/CommStream.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/BMIResource.hh"

#include "net/Net.hh"
#include "net/bmi/BMIConnector.hh"

namespace iofwdclient
{
   //========================================================================

   CommStream::CommStream ()
   {

   }
//   CommStream::CommStream (const char * forwarder)
//      : bmiresource_ (new iofwdevent::BMIResource ()),
//        net_ (new net::bmi::BMIConnector (*bmiresource_,
//                 iofwdutil::IOFWDLog::getSource ()))
//   {
//      setDestination (forwarder);
//   }

//   void CommStream::setDestination (const char * dest)
//   {
//      iofwdevent::SingleCompletion block;   // temporary

//      net_->lookup (dest, &dest_, block);
//      // problem here: we're supposed to drive the poll loop
//      //   - call poll framework (doesn't exist yet) to make progress
//      //   - or: make it into a request and wait on request?
//      //
//      block.wait ();
//   }

//   rpc::RPCClientHandle CommStream::connect (rpc::RPCKey k)
//   {
//      return rpc::RPCClient::rpcConnect (k, net_->connect (dest_));
//   }

   //========================================================================
}
