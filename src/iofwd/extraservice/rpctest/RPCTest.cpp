#include "iofwd/extraservice/rpctest/RPCTest.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "encoder/xdr/XDRReader.hh"

#include "rpc/RPCInfo.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

SERVICE_REGISTER(iofwd::extraservice::RPCTest, rpctest);

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
         static void threadRPC (const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
            boost::thread (boost::bind (h, in, out, info));
         }

         rpc::RPCHandler rpcExec (const rpc::RPCHandler & orig)
         {
            return boost::bind (&threadRPC, orig, _1, _2, _3);
         }

      }

      RPCTest::RPCTest (service::ServiceManager & m)
         : ExtraService (m),
           log_service_ (lookupService<Log> ("log")),
           rpcserver_ (lookupService<RPCServer> ("rpcserver")),
           log_ (log_service_->getSource ("rpctest"))
      {
         rpcserver_->registerRPC ("rpctest.null",
               rpcExec (boost::bind (&RPCTest::null, this, _1, _2, _3)));
         rpcserver_->registerRPC ("rpctest.echo",
               rpcExec (boost::bind (&RPCTest::echo, this, _1, _2, _3)));
      }

      RPCTest::~RPCTest ()
      {
         rpcserver_->unregisterRPC ("rpctest.echo");
         rpcserver_->unregisterRPC ("rpctest.null");
      }

      void RPCTest::null (ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out, const rpc::RPCInfo & )
      {
         delete (in);
         delete (out);
      }

      void RPCTest::echo (ZeroCopyInputStream * i, ZeroCopyOutputStream * o,
            const rpc::RPCInfo &)
      {
         scoped_ptr<ZeroCopyInputStream> in (i);
         scoped_ptr<ZeroCopyOutputStream> out (o);
         const char * read_ptr;
         size_t read_size = 0;
         size_t read_used = 0;
         char * write_ptr;
         size_t write_size = 0;
         size_t write_used = 0;

         SingleCompletion block;
         in->read (reinterpret_cast<const void **>(&read_ptr), &read_size, block, 0);
         block.wait ();

         encoder::xdr::XDRReader dec (read_ptr, read_size);
         uint32_t echo_todo;
         process (dec, echo_todo);
         read_used += dec.getPos ();

         ZLOG_INFO (log_, format("Echo RPC: %u bytes") % echo_todo);

         while (echo_todo)
         {
            if (!read_size || (read_used == read_size))
            {
               block.reset ();
               in->read (reinterpret_cast<const void**>(&read_ptr), &read_size,
                     block, 0);
               block.wait ();
               read_used = 0;
            }
            if (!write_size || (write_used == write_size))
            {
               block.reset ();
               out->write (reinterpret_cast<void**>(&write_ptr), &write_size,
                     block, 0);
               block.wait ();
               write_used = 0;
            }

            const size_t readavail = read_size - read_used;
            const size_t writeavail = write_size - write_used;
            const size_t thiscopy = std::min (readavail, writeavail);

            ASSERT (thiscopy <= echo_todo);

            memcpy (write_ptr + write_used, read_ptr + read_used, thiscopy);
            write_used += thiscopy;
            read_used += thiscopy;
            
            echo_todo -= thiscopy;
         }


         block.reset ();
         out->flush (block);
         block.wait ();
      }

      //=====================================================================
   }
}
