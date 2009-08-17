#include <signal.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "iofwdutil/assert.hh"
#include "zlog/ZLog.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/IOFWDMain.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace po = boost::program_options;

using namespace std; 

// =========== Command Line Options =========================================
static std::string      opt_configfile; 
static unsigned int     opt_debug               = 0; 
static bool             opt_verbose             = 0; 
static bool             opt_notrap              = 0; 
// ==========================================================================

int main (int argc, char ** args)
{

   // Try to activate logging as soon as possible


   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("config", po::value<std::string>(), "specify configuration file")
      ("debug", po::value<unsigned int>(), "set debug level")
      ("verbose", po::value<bool>(), "Enable verbose messages")
      ("notrap", "Don't trap signals (for debugging)")
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

   if (vm.count("notrap"))
      opt_notrap = true; 
   
   // ---- enable logging ---
   iofwdutil::zlog::ZLogSource & mainlog = iofwdutil::IOFWDLog::getSource (); 
   ZLOG_INFO(mainlog, "iofwd main started..."); 
   
   if (opt_notrap)
   {
      ZLOG_INFO(mainlog, "--notrap specified: won't protect threads from"
            " CTRL-C. Use SIGUSR1 for a clean"
            " shutdown"); 
   }
   

   try
   {
      iofwd::IOFWDMain main (opt_notrap); 
      ZLOG_INFO (mainlog, "Booting IOFWDMain..."); 
      main.boot (); 

      //trapSignals (main); 

      ZLOG_INFO (mainlog, "About to run IOFWDMain..."); 
      main.run (); 
      ZLOG_INFO (mainlog, "Shutting down IOFWDMain..."); 
      main.shutdown (); 
   }
   catch  (...)
   {
      ZLOG_ERROR (mainlog, "Unhandled exception propagated to main!");
      cerr << "Exception occured!\n";
      throw; 
   }
   
   ZLOG_INFO(mainlog, "IOFWD exiting..."); 

   return EXIT_SUCCESS; 
}

