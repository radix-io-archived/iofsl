#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/bind.hpp>

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/Singleton.hh"
#include "iofwdutil/assert.hh"

#include "iofwdevent/MultiCompletion.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/ResourceWrapper.hh"

#include "rpc/RPCClientWrapper.hh"
#include "rpc/RPCServerWrapper.hh"
#include "rpc/RPCLocalExec.hh"

#include "rpc/local/LocalConnector.hh"
#include "rpc/RPCServer.hh"
#include "rpc/RPCClient.hh"
#include "rpc/RPCRegistry.hh"

using namespace boost::accumulators;
using namespace std;
using namespace boost::program_options;
using namespace boost;
using namespace iofwdevent;
using namespace rpc;


enum { KB = 1024, MB = 1024*KB };

//===========================================================================

struct Func1
{
   typedef int INPUT;
   typedef int OUTPUT;

   /*static const char * getRPCName ()
   { return "func1"; } */

   static void execute (const INPUT & in, OUTPUT & out)
   {
      out = 2* (in);
      cout << "Execute: in = " << in << ", out=" << out << endl;
   }

};

//===========================================================================

struct RPCTest
{
   virtual bool hasServer () const { return false; }

   virtual void runServer () { ALWAYS_ASSERT(false); }

   virtual void runTest1 () = 0;

   virtual ~RPCTest () {}
};

//===========================================================================


std::string opt_remote;
std::string opt_listen;
size_t      opt_loops;
bool        opt_validate;
bool        opt_local = false;

//===========================================================================

struct BMIServer : public RPCTest
{
   BMIServer (const char * )
   {
   }

   void runTest1 ()
   {
   }

};

//===========================================================================

struct BMIClient : public RPCTest
{
   BMIClient (const char * )
   {
   }

   void runTest1 ()
   {
   }

   RPCClientWrapper<Func1> func1c_;
};

//===========================================================================
struct LocalTest : public RPCTest
{

   void runTest1 ()
   {
      cout << "size of client wrapper: " << sizeof(func1c) << endl;
      cout << "size of server wrapper: " << sizeof(func1s) << endl;
      cout << "query size: " << func1c.getMaxInputSize () << endl;
      cout << "response size: " << func1c.getMaxOutputSize () << endl;
      cout << "key: " << func1c.getKey () << endl;
      cout << "server key: " << func1s.getKey () << endl;
      int in = 2;
      int a;
      cout << "In = " << in << endl;
      localexec (func1c,func1s, comp) (in, a);
      comp.wait ();
      cout << "Out = " << a << endl;
   }

   RPCClientWrapper<Func1> func1c;
   RPCServerWrapper<Func1> func1s;
   RPCLocalExec            localexec;
   SingleCompletion        comp;
};
//===========================================================================

struct Bypass : public RPCTest
{
   Bypass ()
   {
      connector_.registerExec (&server_);
      func1s.registerKey ();
   }

   void runTest1 ()
   {
      iofwdevent::SingleCompletion comp;

      int in = 2;
      int a;
      cout << "In = " << in << endl;
      func1c.bind (2, a);
      RPCClient::execute (comp, connector_, &func1c);
      comp.wait ();
      cout << "Out = " << a << endl;
   }

   RPCClientWrapper<Func1> func1c;
   RPCServerWrapper<Func1> func1s;
   RPCClient client_;
   RPCServer server_;
   rpc::local::LocalConnector connector_;
};

//===========================================================================

void runTest (RPCTest * test)
{
   test->runTest1 ();
}

//===========================================================================

int main (int argc, char ** args)
{
   options_description desc ("Options");
   desc.add_options ()
      ("help", "Show program usage")
      ("loops", value<size_t>(&opt_loops)->default_value(100),
          "Number of iterations for each message size")
      ("remote", value<std::string>(&opt_remote), "Remote server to connect to")
      ("local", "Run loopback RPC test")
      ("bypass", "Run encode/decode without sending/receiving data")
      ("listen", value<std::string>(&opt_listen), "Which address to listen on")
      ("validate",value<bool>(&opt_validate), "Validate received data")
      ;

   variables_map vm;
   store(command_line_parser(argc, args).options(desc).run(),
         vm);
   notify(vm);

   if (vm.count("help"))
   {
      cout << desc << "\n";
      return 1;
   }

   opt_local = vm.count("local");

   if ( ((opt_remote.empty() ? 0 : 1) 
          + (opt_listen.empty() ? 0 : 1)
          + (vm.count("local") ? 1 : 0)
          + (vm.count("bypass") ? 1 : 0)
         ) != 1)
   {
      cerr << "Need to specify one of --bypass,--listen,--remote,--local!\n";
      return 1;
   }

   boost::scoped_ptr<RPCTest> test;

   if (!opt_remote.empty())
   {
      test.reset (new BMIServer (opt_listen.c_str()));
   }
   else if (!opt_listen.empty())
   {
      test.reset (new BMIClient (opt_remote.c_str()));
   }
   else if (vm.count("local"))
   {
      test.reset (new LocalTest ());
   }
   else
   {
      test.reset (new Bypass ());
   }

   if (test->hasServer ())
   {
      test->runServer ();
   }
   else
   {
      runTest (test.get());
   }
}
