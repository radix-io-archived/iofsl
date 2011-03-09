#include "iofwd/extraservice/aarpc/AtomicAppendRPC.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "encoder/xdr/XDRReader.hh"

#include "rpc/RPCInfo.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include <iostream>

SERVICE_REGISTER(iofwd::extraservice::AtomicAppendRPC, aarpc);

using namespace iofwdevent;
using namespace boost;

namespace iofwd
{
   namespace extraservice
   {
      //=====================================================================

      namespace
      {

         /**
          * Conveniently create a thread to handle the RPC call
          */
         static void threadRPC(const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
            boost::thread(boost::bind(h, in, out, info));
         }

         rpc::RPCHandler rpcExec(const rpc::RPCHandler & orig)
         {
            return boost::bind(&threadRPC, orig, _1, _2, _3);
         }

      }

      AtomicAppendRPC::AtomicAppendRPC(service::ServiceManager & m)
         : ExtraService (m),
           log_service_ (lookupService<Log>("log")),
           rpcserver_ (lookupService<RPCServer>("rpcserver")),
           log_ (log_service_->getSource("aarpc"))
      {
         rpcserver_->registerRPC("aarpc.getnextoffset",
               rpcExec(boost::bind(&AtomicAppendRPC::getNextOffset, this, _1, _2, _3)));
      }

      AtomicAppendRPC::~AtomicAppendRPC()
      {
         rpcserver_->unregisterRPC("aarpc.getnextoffset");
      }

      void AtomicAppendRPC::getNextOffset(ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          boost::scoped_ptr<ZeroCopyInputStream> instream(in);
          boost::scoped_ptr<ZeroCopyOutputStream> outstream(out);
      }
      //=====================================================================
   }
}
