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

            void getattr(iofwdevent::ZeroCopyInputStream * in,
                    iofwdevent::ZeroCopyOutputStream * out,
                    const rpc::RPCInfo & );

            void lookup(iofwdevent::ZeroCopyInputStream * in,
                    iofwdevent::ZeroCopyOutputStream * out,
                    const rpc::RPCInfo & );

         protected:
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;

            boost::scoped_ptr<iofwd::tasksm::TaskSMFactory> sm_factory_;
      };
   }
}

#endif
