#ifndef EXTRASERVICE_AARPC_ATOMICAPPENDRPC_HH
#define EXTRASERVICE_AARPC_ATOMICAPPENDRPC_HH

#include "iofwd/ExtraService.hh"
#include "rpc/RPCHandler.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

namespace iofwd
{
   //========================================================================

   class Log;
   class RPCServer;

   namespace extraservice
   {
      //=====================================================================


      class AtomicAppendRPC : public ExtraService
      {
         public:
            AtomicAppendRPC(service::ServiceManager & m);

            virtual void configureNested(const iofwdutil::ConfigFile &) {}

            virtual ~AtomicAppendRPC();

         protected:
            void getNextOffset(iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

         protected:
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;
      };

      //=====================================================================
   }

   //========================================================================
}

#endif
