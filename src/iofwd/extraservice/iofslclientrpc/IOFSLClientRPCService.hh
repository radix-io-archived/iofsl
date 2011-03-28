#ifndef EXTRASERVICE_CLIENTRPC_IOFSLCLIENTRPCSERVICE_HH
#define EXTRASERVICE_CLIENTRPC_IOFSLCLIENTRPCSERVICE_HH

#include "rpc/RPCHandler.hh"

#include "zoidfs/zoidfs.h"

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdutil/tools.hh"

#include "iofwd/ExtraService.hh"
#include "iofwd/tasksm/TaskSMFactory.hh"

#include <boost/shared_ptr.hpp>

#define RPC_GENCLIENTHEADERS(CLASSNAME, RPCNAME)                             \
      void RPCNAME   (iofwdevent::ZeroCopyInputStream * in,                  \
                    iofwdevent::ZeroCopyOutputStream * out,                  \
                    const rpc::RPCInfo & );
namespace iofwd
{
   class Log;
   class RPCServer;

   namespace extraservice
   {
      class IOFSLClientRPCService : public ExtraService
      {
         public:
            IOFSLClientRPCService(service::ServiceManager & m);

            virtual void configureNested(const iofwdutil::ConfigFile & UNUSED(f))
            {
            }

            virtual ~IOFSLClientRPCService();

         protected:

            /* rpc handlers */

            RPC_GENCLIENTHEADERS (IOFSLRPCCommitRequest, commit)
            RPC_GENCLIENTHEADERS (IOFSLRPCCreateRequest, create)
            RPC_GENCLIENTHEADERS (IOFSLRPCGetAttrRequest, getattr)
            RPC_GENCLIENTHEADERS (IOFSLRPCLinkRequest, link)
            RPC_GENCLIENTHEADERS (IOFSLRPCLookupRequest, lookup)
            RPC_GENCLIENTHEADERS (IOFSLRPCMkdirRequest, mkdir)
            RPC_GENCLIENTHEADERS (IOFSLRPCNotImplementedRequest, notimplemented)
            RPC_GENCLIENTHEADERS (IOFSLRPCNullRequest, null)
            RPC_GENCLIENTHEADERS (IOFSLRPCReadDirRequest, readdir)
            RPC_GENCLIENTHEADERS (IOFSLRPCReadLinkRequest, readlink)
            RPC_GENCLIENTHEADERS (IOFSLRPCReadRequest, read)
            RPC_GENCLIENTHEADERS (IOFSLRPCRemoveRequest, remove)
            RPC_GENCLIENTHEADERS (IOFSLRPCRenameRequest, rename)
            RPC_GENCLIENTHEADERS (IOFSLRPCResizeRequest, resize)
            RPC_GENCLIENTHEADERS (IOFSLRPCSetAttrRequest, setattr)
            RPC_GENCLIENTHEADERS (IOFSLRPCSymLinkRequest, symlink)
            RPC_GENCLIENTHEADERS (IOFSLRPCWriteRequest, write)


//            void getattr(iofwdevent::ZeroCopyInputStream * in,
//                    iofwdevent::ZeroCopyOutputStream * out,
//                    const rpc::RPCInfo & );

//            void lookup(iofwdevent::ZeroCopyInputStream * in,
//                    iofwdevent::ZeroCopyOutputStream * out,
//                    const rpc::RPCInfo & );

         protected:
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<iofwd::RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;

            boost::scoped_ptr<iofwd::tasksm::TaskSMFactory> sm_factory_;
      };
   }
}

#endif
