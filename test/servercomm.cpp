#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/RPCServer.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"

#include "net/Communicator.hh"
#include "net/Net.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include <string>
#include <iostream>
#include <unistd.h>

#include <boost/thread.hpp>
#include <boost/program_options.hpp>

using namespace iofwd;
using namespace boost;
using namespace iofwd::service;
using namespace std;
using namespace iofwdevent;
using namespace rpc;
using namespace boost::program_options;

iofwdutil::IOFWDLogSource & log_ = iofwdutil::IOFWDLog::getSource ();

/**
 * Conveniently create a thread to handle the RPC call
 */
void threadRPC (const rpc::RPCHandler & h,
      ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
      const rpc::RPCInfo & info)
{
   boost::thread (boost::bind (h, in, out, info));
}

void rpc_null (ZeroCopyInputStream * i, ZeroCopyOutputStream * o,
      const rpc::RPCInfo & )
{
   boost::scoped_ptr<ZeroCopyInputStream> in (i);
   boost::scoped_ptr<ZeroCopyOutputStream> out (o);

   ZLOG_INFO (log_, "incoming rpc_null call...");

   // need to flush out
   SingleCompletion block;
   out->flush (block);
   block.wait ();

}

void rpc_echo (ZeroCopyInputStream * i, ZeroCopyOutputStream * o,
      const rpc::RPCInfo & )
{
   boost::scoped_ptr<ZeroCopyInputStream> in (i);
   boost::scoped_ptr<ZeroCopyOutputStream> out (o);

   ZLOG_INFO (log_, "incoming rpc_echo call...");

   // Decode number of bytes
   SingleCompletion block;

   const void * read_ptr;
   size_t read_size;
   void * write_ptr;
   size_t write_size;

   block.reset ();
   in->read (&read_ptr, &read_size, block, 0);
   block.wait ();

   rpc::RPCDecoder dec (read_ptr, read_size);
   uint32_t bytes;
   process (dec, bytes);

   block.reset ();
   // Put back unused bytes
   in->rewindInput (read_size - dec.getPos (), block);
   block.wait ();

   size_t todo = bytes;

   while (todo)
   {
      // Obtain read buffer
      block.reset ();
      in->read (&read_ptr, &read_size, block, 0);
      block.wait ();

      size_t read_used = 0;
      size_t write_used = 0;
      while (read_used != read_size)
      {
         block.reset ();
         out->write (&write_ptr, &write_size, block, read_size - read_used);
         block.wait ();

         const size_t thistransfer = std::min (write_size, read_size -
               read_used);
         memcpy (static_cast<char*>(write_ptr)+write_used,
               static_cast<const char*>(read_ptr)+read_used,thistransfer);
         write_used += thistransfer;
         read_used += thistransfer;
         todo -= thistransfer;
      }

      block.reset ();
      out->rewindOutput (write_size - write_used, block);
      block.wait ();
   }
}

template <typename IN, typename OUT>
void rpcServerHelper (const boost::function<void (const IN &, OUT &)> & func,
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
}

// note const int &  for IN, int & for OUT
static void ping (size_t myrank, const int & in, int & out)
{
   out = myrank;
   ZLOG_INFO (log_, format("Ping from %i, out=%i...") % in % out);
}

int main (int argc, char ** args)
{
   try
   {
      std::string opt_config;

      options_description desc ("Options");
      desc.add_options ()
         ("help", "Show program usage")
         ("config",
          value<std::string>(&opt_config)->default_value("defaultconfig.cf"),
          "Config file to use")
         ;

      variables_map vm;
      store(command_line_parser(argc, args).options(desc).run(),
            vm);
      notify(vm);

      if (vm.count ("help"))
      {
         std::cout << desc << "\n";
         return 0;
      }

      // Needed to autoregister services
      registerIofwdFactoryClients ();

      ServiceManager & man = ServiceManager::instance ();

      man.setParam ("config.configfile", opt_config);


      // Defined here so the server keeps running until the client is done.
      shared_ptr<iofwd::RPCServer> rpcserver;
      shared_ptr<iofwd::Net> netservice (man.loadService<iofwd::Net>("net"));
      // net::Net * net = netservice->getNet ();
      net::ConstCommunicatorHandle c = netservice->getServerComm ();
      const size_t myrank = c->rank ();


      rpcserver = (man.loadService<iofwd::RPCServer>("rpcserver"));

      boost::function<void (const int &, int &)> tmp2 =
         bind (ping, myrank, _1, _2);
      // For clarity (better to avoid temporaries like this)
      RPCHandler tmp = boost::bind (&rpcServerHelper<int,int>,
            tmp2,
            _1, _2, _3);

      // Register RPC handlers

      rpcserver->registerRPC ("user.ping",
            boost::bind (&threadRPC, tmp, _1, _2, _3));


      shared_ptr<iofwd::RPCClient> rpcclient
         (man.loadService<iofwd::RPCClient>("rpcclient"));


      // Lookup RPC destination
      SingleCompletion block;

      ZLOG_INFO (log_, format("server comm: rank %i of %i")
            % c->rank () % c->size ());

      for (size_t i=0; i<c->size (); ++i)
      {
         int in;
         int out;

         in = c->rank ();
         rpcClientHelper (in, out,
               rpcclient->rpcConnect ("user.ping", (*c)[i]));
         ZLOG_INFO (log_, format ("user.ping: in=%i out=%i") % in % out);
      }

      ZLOG_INFO (log_, "All responded!");


      cerr << "Press enter to stop RPC server...\n";
      std::string dummy;
      getline (cin, dummy);
   }
   catch (std::exception & e)
   {
      cerr << boost::diagnostic_information (e);
   }
}

