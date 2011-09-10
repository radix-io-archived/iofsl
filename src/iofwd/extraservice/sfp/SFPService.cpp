#include "iofwd/extraservice/sfp/SFPService.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "encoder/xdr/XDRReader.hh"

#include "rpc/RPCInfo.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

SERVICE_REGISTER(iofwd::extraservice::SFPService, sfp);

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
         /*static void threadRPC (const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
            boost::thread (boost::bind (h, in, out, info));
         }

         rpc::RPCHandler rpcExec (const rpc::RPCHandler & orig)
         {
            return boost::bind (&threadRPC, orig, _1, _2, _3);
         }


         template <typename IN, typename OUT>
            void rpcServerHelper (boost::function<void (const IN &, OUT &)> & func,
                  ZeroCopyInputStream * i,
                  ZeroCopyOutputStream * o,
                  const RPCInfo & )
            {
               scoped_ptr<ZeroCopyInputStream> in (i);
               scoped_ptr<ZeroCopyOutputStream> out (o);

               IN arg_in;
               OUT arg_out;
               SingleCompletion block;

               {
                  const void * read_ptr;
                  size_t read_size;
                  const size_t insize = getRPCEncodedSize (IN()).getMaxSize ();

                  block.reset ();
                  in->read (&read_ptr, &read_size, block, insize);
                  block.wait ();

                  RPCDecoder dec (read_ptr, read_size);
                  process (dec, arg_in);
                  if (dec.getPos () < read_size)
                     ZLOG_ERROR (log_, "RPCServer: extra data after RPC request?");
               }

               // call user func
               func (arg_in, arg_out);

               // Send response
               {
                  void * write_ptr;
                  size_t write_size;
                  const size_t outsize = getRPCEncodedSize (OUT()).getMaxSize ();

                  block.reset ();
                  out->write (&write_ptr, &write_size, block, outsize);
                  block.wait ();

                  RPCEncoder enc (write_ptr, write_size);
                  process (enc, arg_out);

                  block.reset ();
                  out->rewindOutput (write_size - enc.getPos (), block);
                  block.wait ();

                  block.reset ();
                  out->flush (block);
                  block.wait ();
               }
            }

         template <typename IN, typename OUT>
            void rpcClientHelper (const IN & in, OUT & out, RPCClientHandle h)
            {
               SingleCompletion block;

               {
                  // wait until outgoing RPC ready
                  block.reset ();
                  h->waitOutReady (block);
                  block.wait ();

                  // Now we can use the outgoing stream
                  scoped_ptr<ZeroCopyOutputStream> out (h->getOut());

                  // encode input arguments for remote RPC
                  void * write_ptr;
                  size_t write_size;


                  const size_t sendsize = getRPCEncodedSize (IN ()).getMaxSize ();

                  block.reset ();
                  out->write (&write_ptr, &write_size, block, sendsize);
                  block.wait ();

                  RPCEncoder enc (write_ptr, write_size);
                  process (enc, in);

                  block.reset ();
                  out->rewindOutput (write_size - enc.getPos (), block);
                  block.wait ();

                  block.reset ();
                  out->flush (block);
                  block.wait ();
               }

               // Read response
               {
                  block.reset ();
                  h->waitInReady (block);
                  block.wait ();

                  scoped_ptr<ZeroCopyInputStream> in (h->getIn ());

                  const void * read_ptr;
                  size_t read_size;

                  const size_t receivesize = getRPCEncodedSize (OUT ()).getMaxSize ();

                  block.reset ();
                  in->read (&read_ptr, &read_size, block, receivesize);
                  block.wait ();

                  RPCDecoder dec (read_ptr, read_size);
                  process (dec, out);

                  if (dec.getPos () != read_size)
                     ZLOG_ERROR (log_, "Extra bytes at end of RPC response?");
               }
            } */
      }

      SFPService::SFPService (service::ServiceManager & m)
         : ExtraService (m),
           log_service_ (lookupService<Log> ("log")),
           rpcserver_ (lookupService<RPCServer> ("rpcserver")),
           log_ (log_service_->getSource ("sfp"))
      {
        /* rpcserver_->registerRPC ("sfp.modify",
               rpcExec (boost::bind (&SFPService::sfpModify, this, _1, _2, _3))); */
      }

      SFPService::~SFPService ()
      {
      /*   rpcserver_->unregisterRPC ("sfp.modify"); */
      }

      void SFPService::configureNested (const iofwdutil::ConfigFile & )
      {
      }

      //=====================================================================
   }
}
