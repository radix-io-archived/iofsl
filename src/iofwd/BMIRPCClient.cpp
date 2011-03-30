#include "BMIRPCClient.hh"

#include "iofwd/service/ServiceManager.hh"

#include "rpc/RPCServer.hh"
#include "net/bmi/BMIConnector.hh"

//#include "rpc/RPCClientWrapper.hh"
#include "rpc/RPCClient.hh"

#include "iofwdevent/SingleCompletion.hh" // temp
#include <unistd.h> // temp

#include "BMI.hh"
#include "Log.hh"
#include "Config.hh"
#include "BMIRPCHelper.hh"

SERVICE_REGISTER(iofwd::BMIRPCClient, bmirpcclient);



namespace iofwd
{
   //========================================================================

   BMIRPCClient::BMIRPCClient (service::ServiceManager & m)
      : service::Service (m),
      log_service_ (lookupService<Log> ("log")),
      config_service_ (lookupService<Config> ("config")),
      bmi_service_ (lookupService<BMI> ("bmi")),
      rpchelper_service_ (lookupService<BMIRPCHelper> ("bmirpchelper")),
      connector_ (rpchelper_service_->getConnector ())
   {
   }

   BMIRPCClient::~BMIRPCClient ()
   {
      // @TODO: ensure we wait until all RPC operations are done
   }


   unsigned int BMIRPCClient::rank () const
   {
      return bmi_service_->getServerRank ();
   }

   unsigned int BMIRPCClient::size () const
   {
      return bmi_service_->getServerCount ();
   }

   rpc::bmi::BMIConnector BMIRPCClient::operator () (const rpc::RPCKey & key,
         unsigned int rank)
   {
      BMI_addr_t addr  = bmi_service_->getServer (rank);
      return connector_ (addr);
   }

  /* void BMIRPCClient::barrier ()
   {
      rpc::RPCClientWrapper<barrierfunc> bf;

      const int myrank = rank ();

      for (unsigned int i=0; i<size(); ++i)
      {
         if (static_cast<int>(i) == myrank)
            continue;

         int in, out;
         bf.bind (in, out);
         iofwdevent::SingleCompletion c;

         unsigned int attempts = 0;
         do
         {
            try
            {
               c.reset ();
               rpc::RPCClient::execute (c, (*this)(i), &bf);
               c.wait ();
            }
            catch (...)
            {
               if (attempts >= 10)
                  throw;

               sleep (1);
               ++attempts;
               continue;
            }
            break;
         }
         while (true);
      }
      
   }  */

   //========================================================================
}
