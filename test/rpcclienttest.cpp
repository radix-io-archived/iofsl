#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/RPCServer.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "net/Net.hh"

#include "iofwd/ExtraService.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"
#include "iofwdclient/LinkHelper.hh"
#include <string>
#include <iostream>
#include <unistd.h>
#include "net/loopback/LoopbackConnector.hh"
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
//#include "iofwdclient/iofwdclientlib.hh"
#include "iofwdclient/CommStream.hh"
#include "iofwdclient/IOFWDClient.hh"
#include "zoidfs/util/ZoidFSHints.hh"
#include "iofwd/extraservice/iofslclientrpc/IOFSLClientRPCService.hh"
using namespace iofwd;
using namespace iofwdevent;
using namespace boost;
using namespace iofwd::service;
using namespace iofwd::extraservice;
using namespace std;
using namespace iofwdevent;
using namespace rpc;
using namespace boost::program_options;
iofwdutil::IOFWDLogSource & log_ = iofwdutil::IOFWDLog::getSource ();

int main (int argc, char ** args)
{
   try
   {
      std::string opt_config;
      std::string opt_remote;
      bool opt_server;

      options_description desc ("Options");
      desc.add_options ()
         ("help", "Show program usage")
         ("config",
          value<std::string>(&opt_config)->default_value("defaultconfig.cf"),
          "Config file to use")
         ("remote", value<std::string>(&opt_remote), "Remote server to connect to")
         ("server", "Be an RPC server")
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
     
      opt_server = vm.count ("server");
      // Needed to autoregister services
      registerIofwdFactoryClients ();
      ServiceManager & man = ServiceManager::instance ();
      man.setParam ("config.configfile", opt_config);


      // Defined here so the server keeps running until the client is done.
      iofwd::extraservice::IOFSLClientRPCService * rpcserver;
      if (opt_server)
      {
         fprintf(stderr, "rpcserver:%s:%i\n", __func__, __LINE__);
          rpcserver = new iofwd::extraservice::IOFSLClientRPCService ( man); 
      }


      if (!opt_remote.empty())
      {
         //net::loopback::LoopbackConnector netservice;
         shared_ptr<iofwd::Net> netservice (man.loadService<iofwd::Net>("net"));
         SingleCompletion block;
         net::Net * net = netservice->getNet ();
         net::AddressPtr addr;
         net->lookup (opt_remote.c_str(), &addr, block);
         block.wait ();
         iofwdclient::IOFWDClient * x  =  new iofwdclient::IOFWDClient (*(new iofwdclient::CommStream()),
                                                                        addr,
                                                                        (bool)true);
         zoidfs::zoidfs_handle_t handle;
         zoidfs::zoidfs_op_hint_t op_hint;
         zoidfs::hints::zoidfs_hint_create(&op_hint);
         size_t ret = 0;
         printf("HANDLE: %i\n", handle);
         printf("MY RETRURN: %i\n",ret);
         ret = x->lookup (NULL, NULL, "/repo/test.txt", &handle, &op_hint);    
                      
          int  _N = 1;
          size_t _BSIZE = 50000;
          size_t mem_sizes[_N]; 
          size_t _foff = 0; 
          size_t mem_count, file_count; 
          uint64_t file_sizes[_N], file_starts[_N]; 
          void *mem_starts_write[_N]; 
          size_t _i = 0; 
          mem_count = _N; 
          file_count = _N; 
          for(_i = 0 ; _i < mem_count ; _i++) 
          { 
              mem_starts_write[_i] = malloc(_BSIZE); 
              memset(mem_starts_write[_i], 'a', _BSIZE); 
              file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
              file_starts[_i] = _foff; 
              _foff += _BSIZE; 
          } 
          ret = x->write (&handle, mem_count, (const void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 
          for(_i = 0 ; _i < mem_count ; _i++) 
          { 
              free(mem_starts_write[_i]); 
          } 
          
          for(_i = 0 ; _i < mem_count ; _i++) 
          { 
              mem_starts_write[_i] = malloc(_BSIZE); 
              file_sizes[_i] = mem_sizes[_i] = _BSIZE; 
              file_starts[_i] = _foff; 
              _foff += _BSIZE; 
          } 

          ret = x->read (&handle, mem_count, (void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT);
          cout << ret;
      }

      if (opt_server)
      {
         cerr << "Press enter to stop RPC server...\n";
         std::string dummy;
         getline (cin, dummy);
      }
   }
   catch (std::exception & e)
   {
      //cerr << boost::diagnosticinformation (e);
   }
}

