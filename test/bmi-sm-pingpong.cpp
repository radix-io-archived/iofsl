#include <iostream>
#include <boost/program_options.hpp>

#include <vector>

#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/TimerResource.hh"

#include "bmi-sm-pingpong-state.hh"

#include "BMIInit.hh"
#include "BMILink.hh"

using namespace boost::program_options;
using namespace boost;

using namespace std;

using namespace test;

static size_t opt_time;
static size_t opt_threads;
static size_t opt_mb;
static size_t opt_count;
static size_t opt_iterations;

int main (int argc, char ** argv)
{

   // Declare the supported options.
   options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("time", value<size_t>(&opt_time)->default_value(0),
                "Run test for specifief number of seconds")
      ("threads", value<size_t>(&opt_threads)->default_value(1),
                "Number of threads driving the state machines")
      ("MB", value<size_t>(&opt_mb)->default_value(0),
                "Stop after sending x MB")
      ("count", value<size_t>(&opt_count)->default_value(1),
                "Number of state machines to run")
      ("endpoint", value< vector<string> >())
      ("iter", value<size_t>(&opt_iterations)->default_value(100),
                "Number of ping-pong iterations per state machine")
      ;
   
   positional_options_description p;
   p.add("endpoint", -1);

   variables_map vm;

   store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
   notify(vm);

   if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
   }

   if (vm.count("endpoint") < 1)
   {
      cerr << "Need one or two bmi addresses...\n";
      return 1;
   }

   const std::vector<std::string> endpoints = vm["endpoint"].as< vector<string> >();
   const std::string n1 = endpoints[0];
   const std::string n2 = endpoints[endpoints.size()-1];
   {
      test::BMIInit init ("bmi_tcp", n1.c_str(), BMI_INIT_SERVER);

      PPInput input (opt_threads, opt_time, opt_mb);



      test::SetupLink link (input.bmi_, n2.c_str());
      link.findEndPoints ();

      BMI_addr_t p1 = link.getP1();
      BMI_addr_t p2 = link.getP2();

      cout << "BMI Addrs = " << p1 << ", " << p2 << endl;

      for (size_t i=0; i<opt_count; ++i)
      {
         input.manager_.schedule (createPingPongSM (input, true, p1, p2, i,
                  opt_iterations));
         input.manager_.schedule (createPingPongSM (input, false, p1, p2, i,
                  opt_iterations));
      }

      input.manager_.startThreads (opt_threads);

      waitPingPongDone ();
      
      input.manager_.stopThreads ();
   }
}
