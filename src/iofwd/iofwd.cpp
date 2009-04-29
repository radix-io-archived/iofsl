#include <iostream>
#include <boost/program_options.hpp>
#include "zlog/ZLog.hh"
#include "iofwdutil/tools.hh"

namespace po = boost::program_options;


int main (int argc, char ** args)
{
   std::string configfile; 

   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("config", po::value<std::string>(), "specify configuration file")
      ("debug", po::value<int>(), "set debug level")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, args, desc), vm);
   po::notify(vm);    

   if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
   }

   if (vm.count("config"))
   {
      configfile = vm["config"].as<std::string>();

   }
   else
   {
      std::cerr << "Need to specify config file!\n"; 
      return 1; 
   }

   return EXIT_SUCCESS; 
}

