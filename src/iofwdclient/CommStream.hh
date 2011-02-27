#ifndef IOFWDCLIENT_COMMSTREAM_HH
#define IOFWDCLIENT_COMMSTREAM_HH

#include "rpc/RPCClient.hh"
#include "net/Net-fwd.hh"
#include "iofwdevent/Resource-fwd.hh"

namespace iofwdclient
{
   //========================================================================

   /**
    * Abstracts communication for the iofwdclient
    */
   class CommStream
   {
      public:
         CommStream (const char * forwarder);

         rpc::RPCClientHandle connect (rpc::RPCKey k);

      protected:
         void setDestination (const char * s);

      protected:
         boost::scoped_ptr<iofwdevent::BMIResource> bmiresource_;
         boost::scoped_ptr<net::Net> net_;
         net::AddressPtr dest_;
   };

   //========================================================================
}

#endif
