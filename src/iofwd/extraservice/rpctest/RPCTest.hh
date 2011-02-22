#ifndef EXTRASERVICE_RPCTEST_RPCTEST_HH
#define EXTRASERVICE_RPCTEST_RPCTEST_HH

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


      class RPCTest : public ExtraService
      {
         public:
            RPCTest (service::ServiceManager & m);

            virtual void configureNested (const iofwdutil::ConfigFile &) {}

            virtual ~RPCTest ();

         protected:
            void null (iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

            void echo (iofwdevent::ZeroCopyInputStream *,
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
