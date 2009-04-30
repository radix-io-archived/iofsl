#include <signal.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "zlog/ZLog.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/IOFWDMain.hh"

namespace po = boost::program_options;

using namespace std; 

// =========== Command Line Options ==================   
static std::string      opt_configfile; 
static unsigned int     opt_debug               = 0; 
static bool             opt_verbose             = 0; 
// ===================================================

void trapSignals ()
{
   sigset_t sigset; 
   sigfillset (&sigset); 
   sigprocmask (SIG_BLOCK, &sigset, 0); 
}


void boot ()
{
}

int main (int argc, char ** args)
{

   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("config", po::value<std::string>(), "specify configuration file")
      ("debug", po::value<unsigned int>(), "set debug level")
      ("verbose", po::value<bool>(), "Enable verbose messages")
      ;

   po::variables_map vm;

   try
   {
      po::store(po::parse_command_line(argc, args, desc), vm);
   }
   catch (const boost::program_options::invalid_option_value & e)
   {
      cerr << "Invalid config option specified: " << e.what() << endl; 
      return 1; 
   }

   po::notify(vm);    

   if (vm.count("help")) {
      std::cout << desc << "\n";
      return EXIT_SUCCESS;
   }

   if (vm.count("config"))
   {
      opt_configfile = vm["config"].as<std::string>();
   }
   else
   {
      std::cerr << "Need to specify config file!\n"; 
      return 1; 
   }

   if (vm.count("debug"))
      opt_debug = vm["debug"].as<unsigned int> (); 

   if (vm.count("verbose"))
      opt_verbose = vm["verbose"].as<bool> (); 

      
   
   iofwd::IOFWDMain main; 

   try
   {
      main.boot (); 
      main.run (); 
      main.shutdown (); 
   }
   catch  (...)
   {
      cerr << "Exception occured!\n";
   }

   return EXIT_SUCCESS; 
}

