#include <signal.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "iofwdutil/assert.hh"
#include "zlog/ZLog.hh"
#include "iofwdutil/tools.hh"
#include "iofwd/IOFWDMain.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdutil/ConfigFile.hh"
#include "c-util/txt_configfile.h"

namespace po = boost::program_options;

using namespace std; 
using namespace iofwdutil;

// =========== Command Line Options =========================================
static std::string      opt_configfile; 
static unsigned int     opt_debug               = 0; 
static bool             opt_verbose             = 0; 
static bool             opt_notrap              = 0; 
static bool             opt_dumpconfig          = false;
static bool             opt_help                = false;
// ==========================================================================

static ConfigFile getConfig (const std::string & name)
{
   char * err = 0;
   ConfigHandle h = txtfile_openConfig (name.c_str(), &err); 
   if (!h || err)
   {
      const std::string s = 
         str(boost::format("Error opening config file '%s': %s") % name % err);
      free (err);
      throw ZException (s);
   }

   return ConfigFile  (h);
}


int main (int argc, char ** args)
{
   // Try to activate logging as soon as possible, we'll reconfigure
   // logging as soon as we get to our config file.
   iofwdutil::zlog::ZLogSource & mainlog = iofwdutil::IOFWDLog::getSource (); 

   try
   {
      po::options_description desc("Allowed options");
      desc.add_options()
         ("help", "produce help message")
         ("config", po::value<std::string>(), "specify configuration file")
         ("debug", po::value<unsigned int>(), "set debug level")
         ("verbose", po::value<bool>(), "Enable verbose messages")
         ("notrap", "Don't trap signals (for debugging)")
         ("dumpconfig", "Read config file and dump to screen (for debugging)")
         ;

      po::variables_map vm;

      try
      {
         po::store(po::parse_command_line(argc, args, desc), vm);
      }
      catch (const boost::program_options::invalid_option_value & e)
      {
         std::cerr <<  format("Invalid config option specified: '%s'") % e.what();
         return 1; 
      }

      po::notify(vm);    

      if (vm.count("debug"))
         opt_debug = vm["debug"].as<unsigned int> (); 

      if (vm.count("verbose"))
         opt_verbose = vm["verbose"].as<bool> (); 

      if (vm.count("notrap"))
         opt_notrap = true; 

      if (vm.count("dumpconfig"))
         opt_dumpconfig = true;

      if (vm.count ("help"))
         opt_help = true;


      if (vm.count("config"))
      {
         opt_configfile = vm["config"].as<std::string>();
      }

      //
      // ------------ Use & check command line options ----------
      // 

      if (opt_help)
      {
         std::cout << desc << "\n";
         return EXIT_SUCCESS;
      }

      if (opt_configfile.empty())
      {
         cerr << "No config file specified (--config)!\n";
         return 1;
      }

      // Try to get to config data so we can configure debugging
      const ConfigFile config_ = getConfig (opt_configfile);

      // Initialize default log level from config file

      if (opt_dumpconfig)
      {
         config_.dumpToStdErr ();
         return EXIT_SUCCESS;
      }

      if (opt_notrap)
      {
         ZLOG_INFO(mainlog, "--notrap specified: won't protect threads from"
               " CTRL-C. Use SIGUSR1 for a clean"
               " shutdown"); 
      }


      iofwd::IOFWDMain main (opt_notrap); 

      ZLOG_INFO (mainlog, "Booting IOFWDMain..."); 
      main.boot (); 

      ZLOG_INFO (mainlog, "About to run IOFWDMain..."); 
      main.run (); 

      ZLOG_INFO (mainlog, "Shutting down IOFWDMain..."); 
      main.shutdown (); 

      ZLOG_INFO(mainlog, "IOFWD exiting..."); 

      return EXIT_SUCCESS; 
   }
   catch (const ZException & e)
   {
      ZLOG_ERROR (mainlog, "Exception occurred:");
      ZLOG_ERROR (mainlog, e.toString ());
      return 1;
   }
   catch  (...)
   {
      ZLOG_ERROR (mainlog, "Unhandled exception propagated to main!");
      cerr << "Exception occured!\n";
      throw; 
   }

   ALWAYS_ASSERT(false && "Should not get here!");
}

