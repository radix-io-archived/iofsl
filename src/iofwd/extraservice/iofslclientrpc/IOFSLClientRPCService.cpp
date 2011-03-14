#include "iofwd/extraservice/iofslclientrpc/IOFSLClientRPCService.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include "iofwd/rpcfrontend/IOFSLRPCGetAttrRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCLookupRequest.hh"

SERVICE_REGISTER(iofwd::extraservice::IOFSLClientRPCService, iofslclientrpc);

namespace iofwd
{
   namespace extraservice
   {
      IOFSLClientRPCService::IOFSLClientRPCService(service::ServiceManager & m)
         : ExtraService(m),
           log_service_(lookupService<Log>("log")),
           rpcserver_(lookupService<RPCServer>("rpcserver")),
           log_(log_service_->getSource("iofslclientrpc"))
      {
         rpcserver_->registerRPC("iofslclientrpc.getattr",
               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.lookup",
               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
      }

      IOFSLClientRPCService::~IOFSLClientRPCService()
      {
         rpcserver_->unregisterRPC("iofslclientrpc.getattr");
         rpcserver_->unregisterRPC("iofslclientrpc.lookup");
      }

      void IOFSLClientRPCService::getattr(iofwdevent::ZeroCopyInputStream * in,
            iofwdevent::ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          /* TODO get the correct op code */
          int opid = 0;

          /* submit the request to the SM factory */
          (*sm_factory_)(new iofwd::rpcfrontend::IOFSLRPCGetAttrRequest(opid,
                      in, out));
      }

      void IOFSLClientRPCService::lookup(iofwdevent::ZeroCopyInputStream * in,
            iofwdevent::ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          /* TODO get the correct op code */
          int opid = 0;

          /* submit the request to the SM factory */
          (*sm_factory_)(new iofwd::rpcfrontend::IOFSLRPCLookupRequest(opid,
                      in, out));
      }
   }
}
