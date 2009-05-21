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
// ==========================================================================

// ============= Global Static Variables ====================================
static iofwd::IOFWDMain *      iofwd_main; 
static bool                    trap_received = 0; 
// ==========================================================================

/**
 * Called when another ctrl-c is detected before the server could shut down
 */
static void forceKill ()
{
   exit (3); 
}


/**
 * Called when on SIGINT
 */
static void signalHandler (int , siginfo_t * , void * )
{
   if (trap_received)
   {
      forceKill (); 
      exit (2); 
   }

   trap_received = 1; 

   ASSERT (iofwd_main); 
   iofwd_main->shutdown (); 
}


static void trapSignals (iofwd::IOFWDMain & main)
{
   iofwd_main = &main; 

   struct sigaction act; 
#ifdef VALGRIND_SAFE
   memset (&act, 0, sizeof(act)); 
#endif
   act.sa_flags = SA_SIGINFO | SA_NODEFER ;
   sigemptyset (&act.sa_mask); 
   if (sigaction (SIGINT, &act, NULL) < 0)
   {
      //  TODO
   }
}


int main (int argc, char ** args)
{

   // Try to activate logging as soon as possible


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

   
   // ---- enable logging ---
   iofwdutil::zlog::ZLogSource & mainlog = iofwdutil::IOFWDLog::getSource (); 
   ZLOG_INFO(mainlog, "iofwd main started..."); 
   
   

   try
   {
      iofwd::IOFWDMain main; 
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

