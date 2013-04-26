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
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/bind/protect.hpp>
#include "iofwd/frontend/rpc/IOFSLRPCLookupRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCCommitRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCCreateRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCGetAttrRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCLinkRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCLookupRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCMkdirRequest.hh"
//#include "iofwd/frontend/rpc/IOFSLRPCNotImplementedRequest.hh"
//#include "iofwd/frontend/rpc/IOFSLRPCNullRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCReadDirRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCReadLinkRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCReadRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCRemoveRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCRenameRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCResizeRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCSetAttrRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCSymLinkRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCWriteRequest.hh"


SERVICE_REGISTER(iofwd::extraservice::IOFSLClientRPCService, iofslclientrpc);

namespace iofwd
{
   namespace extraservice
   {
      IOFSLClientRPCService::IOFSLClientRPCService(service::ServiceManager & m)
         : ExtraService(m),
           log_service_(lookupService<Log>("log")),
           requesthandler_ (lookupService<RequestHandler>("requesthandler")),
           log_(log_service_->getSource("iofslclientrpc")),
           tp_(iofwdutil::ThreadPool::instance())
      {

         rpcserver_ = (m.loadService<iofwd::RPCServer>("rpcserver"));
         rpcserver_->registerRPC("iofslclientrpc.lookup",
               boost::bind(&IOFSLClientRPCService::lookup, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.commit",
               boost::bind(&IOFSLClientRPCService::commit, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.create",
               boost::bind(&IOFSLClientRPCService::create, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.getattr",
               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.link",
               boost::bind(&IOFSLClientRPCService::link, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.mkdir",
               boost::bind(&IOFSLClientRPCService::mkdir, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.getattr",
               boost::bind(&IOFSLClientRPCService::getattr, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.notimplemented",
//               boost::bind(&IOFSLClientRPCService::notimplemented, this, _1, _2, _3));
//         rpcserver_->registerRPC("iofslclientrpc.null",
//               boost::bind(&IOFSLClientRPCService::null, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.readdir",
               boost::bind(&IOFSLClientRPCService::readdir, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.readlink",
               boost::bind(&IOFSLClientRPCService::readlink, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.read",
               boost::bind(&IOFSLClientRPCService::read, this,_1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.remove",
               boost::bind(&IOFSLClientRPCService::remove, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.rename",
               boost::bind(&IOFSLClientRPCService::rename, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.resize",
               boost::bind(&IOFSLClientRPCService::resize, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.setattr",
               boost::bind(&IOFSLClientRPCService::setattr, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.symlink",
               boost::bind(&IOFSLClientRPCService::symlink, this, _1, _2, _3));
         rpcserver_->registerRPC("iofslclientrpc.write",
               boost::bind(&IOFSLClientRPCService::write, this, _1, _2, _3));
      }

      IOFSLClientRPCService::~IOFSLClientRPCService()
      {
        /* change to scope rpc for auto deletion */
         rpcserver_->unregisterRPC("iofslclientrpc.commit");
         rpcserver_->unregisterRPC("iofslclientrpc.create");
         rpcserver_->unregisterRPC("iofslclientrpc.link");
         rpcserver_->unregisterRPC("iofslclientrpc.lookup");
         rpcserver_->unregisterRPC("iofslclientrpc.mkdir");
         rpcserver_->unregisterRPC("iofslclientrpc.getattr");
//         rpcserver_->unregisterRPC("iofslclientrpc.notimplemented");
//         rpcserver_->unregisterRPC("iofslclientrpc.null");
         rpcserver_->unregisterRPC("iofslclientrpc.readdir");
         rpcserver_->unregisterRPC("iofslclientrpc.readlink");
         rpcserver_->unregisterRPC("iofslclientrpc.read");
         rpcserver_->unregisterRPC("iofslclientrpc.remove");
         rpcserver_->unregisterRPC("iofslclientrpc.rename");
         rpcserver_->unregisterRPC("iofslclientrpc.resize");
         rpcserver_->unregisterRPC("iofslclientrpc.setattr");
         rpcserver_->unregisterRPC("iofslclientrpc.symlink");
         rpcserver_->unregisterRPC("iofslclientrpc.write");
      }                                

      void IOFSLClientRPCService::submitRequest ( iofwd::Request * r)
      {
          requesthandler_->handleRequest ( 1, &r);          
      }

      void IOFSLClientRPCService::write (iofwdevent::ZeroCopyInputStream * in, 
                                         iofwdevent::ZeroCopyOutputStream * out, 
                                         const rpc::RPCInfo & )
      {
          
          /* TODO get the correct op code */                                     
          int opid = zoidfs::ZOIDFS_PROTO_WRITE;                                  
                                                                                 
          iofwd::Request * tmp = new iofwd::rpcfrontend::IOFSLRPCWriteRequest(opid,         
                                                                              in, out);
          iofwdevent::CBType submitReq = boost::bind(&IOFSLClientRPCService::submitRequest, this, tmp);
          static_cast<iofwd::rpcfrontend::IOFSLRPCReadRequest *>(tmp)->decode(submitReq);
      }


      void IOFSLClientRPCService::read ( iofwdevent::ZeroCopyInputStream * in, 
                                         iofwdevent::ZeroCopyOutputStream * out, 
                                         const rpc::RPCInfo & )
      {
          /* TODO get the correct op code */                                     
          int opid = zoidfs::ZOIDFS_PROTO_READ;                                  
                                                                                 
          iofwd::Request * tmp = new iofwd::rpcfrontend::IOFSLRPCReadRequest( opid,         
                                                                              in, out);
          iofwdevent::CBType submitReq = boost::bind(&IOFSLClientRPCService::submitRequest, this, tmp);
          static_cast<iofwd::rpcfrontend::IOFSLRPCReadRequest *>(tmp)->decode(submitReq);
      }


#define RPC_GENCLIENTCODE(CLASSNAME, RPCNAME, OPID)                              \
      void IOFSLClientRPCService::RPCNAME (iofwdevent::ZeroCopyInputStream * in, \
            iofwdevent::ZeroCopyOutputStream * out, const rpc::RPCInfo & )       \
      {                                                                          \
          /* TODO get the correct op code */                                     \
          int opid = OPID;                                                       \
                                                                                 \
          iofwd::Request * tmp = new iofwd::rpcfrontend::CLASSNAME(opid,         \
                                                            in, out);            \
          iofwdevent::CBType submitReq = boost::bind(&IOFSLClientRPCService::submitRequest, this, tmp); \
          static_cast<iofwd::rpcfrontend::CLASSNAME *>(tmp)->decode(submitReq);               \
      }   


      RPC_GENCLIENTCODE (IOFSLRPCCommitRequest, commit, zoidfs::ZOIDFS_PROTO_COMMIT)
      RPC_GENCLIENTCODE (IOFSLRPCCreateRequest, create, zoidfs::ZOIDFS_PROTO_CREATE)
      RPC_GENCLIENTCODE (IOFSLRPCGetAttrRequest, getattr, zoidfs::ZOIDFS_PROTO_GET_ATTR)
      RPC_GENCLIENTCODE (IOFSLRPCLinkRequest, link, zoidfs::ZOIDFS_PROTO_LINK)
      RPC_GENCLIENTCODE (IOFSLRPCLookupRequest, lookup, zoidfs::ZOIDFS_PROTO_LOOKUP)
      RPC_GENCLIENTCODE (IOFSLRPCMkdirRequest, mkdir, zoidfs::ZOIDFS_PROTO_MKDIR)
//      RPC_GENCLIENTCODE (IOFSLRPCNotImplementedRequest, notimplemented, zoidfs::ZOIDFS_PROTO_NULL)
//      RPC_GENCLIENTCODE (IOFSLRPCNullRequest, null, zoidfs::ZOIDFS_PROTO_NULL)
      RPC_GENCLIENTCODE (IOFSLRPCReadDirRequest, readdir, zoidfs::ZOIDFS_PROTO_READDIR)
      RPC_GENCLIENTCODE (IOFSLRPCReadLinkRequest, readlink, zoidfs::ZOIDFS_PROTO_READLINK)
      RPC_GENCLIENTCODE (IOFSLRPCRemoveRequest, remove, zoidfs::ZOIDFS_PROTO_REMOVE)
      RPC_GENCLIENTCODE (IOFSLRPCRenameRequest, rename, zoidfs::ZOIDFS_PROTO_RENAME)
      RPC_GENCLIENTCODE (IOFSLRPCResizeRequest, resize, zoidfs::ZOIDFS_PROTO_RESIZE)
      RPC_GENCLIENTCODE (IOFSLRPCSetAttrRequest, setattr, zoidfs::ZOIDFS_PROTO_SET_ATTR)
      RPC_GENCLIENTCODE (IOFSLRPCSymLinkRequest, symlink, zoidfs::ZOIDFS_PROTO_SYMLINK)
   }
}
