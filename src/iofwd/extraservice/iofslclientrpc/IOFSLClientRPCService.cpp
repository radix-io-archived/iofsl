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
#include "iofwd/rpcfrontend/IOFSLRPCLookupRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCCommitRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCCreateRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCGetAttrRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCLinkRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCLookupRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCMkdirRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCNotImplementedRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCNullRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCReadDirRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCReadLinkRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCReadRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCRemoveRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCRenameRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCResizeRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCSetAttrRequest.hh"
//#include "iofwd/rpcfrontend/IOFSLRPCSymLinkRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCWriteRequest.hh"


SERVICE_REGISTER(iofwd::extraservice::IOFSLClientRPCService, iofslclientrpc);

namespace iofwd
{
   namespace extraservice
   {
      IOFSLClientRPCService::IOFSLClientRPCService(service::ServiceManager & m)
         : ExtraService(m),
           log_service_(lookupService<Log>("log")),
           requesthandler_ (lookupService<RequestHandler>("requesthandler")),
           log_(log_service_->getSource("iofslclientrpc"))
      {
         rpcserver_ = (m.loadService<iofwd::RPCServer>("rpcserver"));
         rpcserver_->registerRPC("iofslclientrpc.lookup",
               boost::bind(&IOFSLClientRPCService::lookup, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.commit",
//               boost::bind(&IOFSLClientRPCService::commit, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.create",
//               boost::bind(&IOFSLClientRPCService::create, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.getattr",
//               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.link",
//               boost::bind(&IOFSLClientRPCService::link, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.mkdir",
//               boost::bind(&IOFSLClientRPCService::mkdir, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.getattr",
//               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.notimplemented",
//               boost::bind(&IOFSLClientRPCService::notimplemented, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.null",
//               boost::bind(&IOFSLClientRPCService::null, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.readdir",
//               boost::bind(&IOFSLClientRPCService::readdir, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.readlink",
//               boost::bind(&IOFSLClientRPCService::readlink, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.read",
//               boost::bind(&IOFSLClientRPCService::read, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.remove",
//               boost::bind(&IOFSLClientRPCService::remove, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.rename",
//               boost::bind(&IOFSLClientRPCService::rename, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.resize",
//               boost::bind(&IOFSLClientRPCService::resize, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.setattr",
//               boost::bind(&IOFSLClientRPCService::setattr, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.symlink",
//               boost::bind(&IOFSLClientRPCService::symlink, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.write",
               boost::bind(&IOFSLClientRPCService::write, this, _1, _2, _3));
      }

      IOFSLClientRPCService::~IOFSLClientRPCService()
      {
        /* change to scope rpc for auto deletion */
//         rpcserver_->unregisterRPC("iofslclientrpc.commit");
//         rpcserver_->unregisterRPC("iofslclientrpc.create");
//         rpcserver_->unregisterRPC("iofslclientrpc.getattr");
//         rpcserver_->unregisterRPC("iofslclientrpc.link");
         rpcserver_->unregisterRPC("iofslclientrpc.lookup");
//         rpcserver_->unregisterRPC("iofslclientrpc.mkdir");
//         rpcserver_->unregisterRPC("iofslclientrpc.getattr");
//         rpcserver_->unregisterRPC("iofslclientrpc.notimplemented");
//         rpcserver_->unregisterRPC("iofslclientrpc.null");
//         rpcserver_->unregisterRPC("iofslclientrpc.readdir");
//         rpcserver_->unregisterRPC("iofslclientrpc.readlink");
//         rpcserver_->unregisterRPC("iofslclientrpc.read");
//         rpcserver_->unregisterRPC("iofslclientrpc.remove");
//         rpcserver_->unregisterRPC("iofslclientrpc.rename");
//         rpcserver_->unregisterRPC("iofslclientrpc.resize");
//         rpcserver_->unregisterRPC("iofslclientrpc.setattr");
//         rpcserver_->unregisterRPC("iofslclientrpc.symlink");
         rpcserver_->unregisterRPC("iofslclientrpc.write");
      }
  
/* Needs request handler */
#define RPC_GENCLIENTCODE(CLASSNAME, RPCNAME, OPID)                              \
      void IOFSLClientRPCService::RPCNAME (iofwdevent::ZeroCopyInputStream * in, \
            iofwdevent::ZeroCopyOutputStream * out, const rpc::RPCInfo & )       \
      {                                                                          \
          /* TODO get the correct op code */                                     \
          int opid = OPID;                                                       \
                                                                                 \
          iofwd::Request * tmp = new iofwd::rpcfrontend::CLASSNAME(opid,         \
                                                            in, out);            \
          requesthandler_->handleRequest ( 1, &tmp);                             \
      }                                   

//      RPC_GENCLIENTCODE (IOFSLRPCCommitRequest, commit)
//      RPC_GENCLIENTCODE (IOFSLRPCCreateRequest, create)
//      RPC_GENCLIENTCODE (IOFSLRPCGetAttrRequest, getattr)
//      RPC_GENCLIENTCODE (IOFSLRPCLinkRequest, link)
      RPC_GENCLIENTCODE (IOFSLRPCLookupRequest, lookup, zoidfs::ZOIDFS_PROTO_LOOKUP)
//      RPC_GENCLIENTCODE (IOFSLRPCMkdirRequest, mkdir)
//      RPC_GENCLIENTCODE (IOFSLRPCNotImplementedRequest, notimplemented)
//      RPC_GENCLIENTCODE (IOFSLRPCNullRequest, null)
//      RPC_GENCLIENTCODE (IOFSLRPCReadDirRequest, readdir)
//      RPC_GENCLIENTCODE (IOFSLRPCReadLinkRequest, readlink)
//      RPC_GENCLIENTCODE (IOFSLRPCReadRequest, read)
//      RPC_GENCLIENTCODE (IOFSLRPCRemoveRequest, remove)
//      RPC_GENCLIENTCODE (IOFSLRPCRenameRequest, rename)
//      RPC_GENCLIENTCODE (IOFSLRPCResizeRequest, resize)
//      RPC_GENCLIENTCODE (IOFSLRPCSetAttrRequest, setattr)
//      RPC_GENCLIENTCODE (IOFSLRPCSymLinkRequest, symlink)
      RPC_GENCLIENTCODE (IOFSLRPCWriteRequest, write, zoidfs::ZOIDFS_PROTO_WRITE)

//      void IOFSLClientRPCService::getattr(iofwdevent::ZeroCopyInputStream * in,
//            iofwdevent::ZeroCopyOutputStream * out,
//            const rpc::RPCInfo & )
//      {
//          /* TODO get the correct op code */
//          int opid = 0;

//          /* submit the request to the SM factory */
//          (*sm_factory_)(new iofwd::rpcfrontend::IOFSLRPCGetAttrRequest(opid,
//                      in, out));
//      }

//      void IOFSLClientRPCService::lookup(iofwdevent::ZeroCopyInputStream * in,
//            iofwdevent::ZeroCopyOutputStream * out,
//            const rpc::RPCInfo & )
//      {
//          /* TODO get the correct op code */
//          int opid = 0;

//          /* submit the request to the SM factory */
//          (*sm_factory_)(new iofwd::rpcfrontend::IOFSLRPCLookupRequest(opid,
//                      in, out));
//      }
   }
}
