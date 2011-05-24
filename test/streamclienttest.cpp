#include "test/streamclienttest.hh"
namespace po = boost::program_options;
iofwdclient::IOFWDClient * iofslSetup( char * address, std::string opt_config)
{
   // Needed to autoregister services
   registerIofwdFactoryClients ();
   registerIofwdutilFactoryClients (); 
   iofwd::service::ServiceManager & man = iofwd::service::ServiceManager::instance ();
   man.setParam ("config.configfile", opt_config);
   boost::shared_ptr<iofwd::Net> netservice (man.loadService<iofwd::Net>("net"));
   iofwdevent::SingleCompletion block;
   net::Net * net = netservice->getNet ();
   net::AddressPtr addr;
   net->lookup (address, &addr, block);
   block.wait ();
   iofwdclient::IOFWDClient * x  =  new iofwdclient::IOFWDClient (*(new iofwdclient::CommStream()),
                                                                  addr,
                                                                  (bool)true);
   return x;
}

int lookupTest(iofwdclient::IOFWDClient * client, std::string dir)
{
  zoidfs::zoidfs_handle_t pathHandle;
  zoidfs::zoidfs_handle_t fullFileHandle;
  zoidfs::zoidfs_handle_t tmpHandle;
  std::ofstream testFile;
  char * fname = new char[dir.length() + 18];
  strcpy(fname, dir.c_str());
  strcat(fname, "lookuptest.txt");
  testFile.open (fname);
  testFile.close();
  zoidfs::zoidfs_op_hint_t op_hint;
  zoidfs::hints::zoidfs_hint_create(&op_hint);
  client->lookup (NULL, NULL, fname, &fullFileHandle, &op_hint);
  client->lookup (NULL, NULL, dir.c_str(), &pathHandle, &op_hint);
  client->lookup (&pathHandle, "lookuptest.txt",NULL, &tmpHandle, &op_hint);
  for (int x = 0; x < 32; x++)
    if (tmpHandle.data[x] != fullFileHandle.data[x])
    {
      std::cout << "Error with lookup, Full path lookup and handle lookup do not match at position " << x << std::endl;
      return -1;
    }
  return 0;
}

int runTest(std::string r, std::string s, std::string c, std::string d)
{
  /* Setup client */
  iofwdclient::IOFWDClient * client = iofslSetup((char*)s.c_str(), c);
  if (strcmp(r.c_str(), "lookup") == 0)
    return lookupTest(client, d);
  std::cout << "No valid request selected for testing" << std::endl;
  return -1;

}

int main (int argc, char *argv[])
{
   // Declare the supported options.
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("list", "List registered transforms")
      ("request", po::value<std::string>(), "Name of request to test (read,write,lookup,ect)")
      ("verbose", "Verbose operation")
      ("server", po::value<std::string>(), "IOFSL Server Address (ex: tcp://127.0.0.1:9001)")
      ("config", po::value<std::string>(), "Client Configuration File")
      ("dir", po::value<std::string>(), "Test Directory")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);
   if (vm.count("help"))
   {
      std::cout << desc << std::endl;
      return 0;
   }  
   if (vm.count("request") == 0)
   {
      std::cout << "Missing request flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("server") == 0)
   {
      std::cout << "Missing server flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("config") == 0)
   {
      std::cout << "Missing config flag, please see --help or --list for parameter information"
                << std::endl;
      return -1;
   }
   if (vm.count("dir") == 0)
   {
      std::cout << "Missing dir flag, please see --help or --list for parameter information" 
                << std::endl;
      return -1;
   }

   std::string request = vm["request"].as<std::string>();
   std::string server = vm["server"].as<std::string>();
   std::string config = vm["config"].as<std::string>();
   std::string dir = vm["dir"].as<std::string>();
   return runTest(request, server, config, dir);
}