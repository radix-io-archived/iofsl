#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

#include "iofwdutil/hash/HashFactory.hh"
#include "iofwdutil/hash/SHA1Simple.hh"
#include "iofwdutil/hash/HashAutoRegister.hh"

using namespace boost;
using namespace iofwdutil::hash;
using namespace std;

namespace po = boost::program_options;

bool opt_verbose = false;


void doList ()
{
   cout << "Available hash functions:\n";
   HashFactory::const_iterator I = HashFactory::instance().begin();
   HashFactory::const_iterator E = HashFactory::instance().end();
   for (size_t pos = 1 ; I!= E; ++I,++pos)
   {
      cout << boost::format("%i. %s (prio %i)") % pos % I->first % I->second
         << endl;
   }
}

void dumpHex (const char * ptr, size_t size)
{
   for (size_t i=0; i<size; ++i)
   {
      // need to cast to unsigned first to avoid sign extension
      cout << boost::format("%02x") % (unsigned) ((unsigned char) ptr[i]);
   }
   cout << endl;
}

void processStdIn (HashFunc * hash)
{
   char buf[4096];
   while (!cin.eof())
   {
      cin.read(&buf[0], sizeof(buf));
      size_t count = cin.gcount();
      if (opt_verbose)
      {
         cerr << "Processing " << count << " bytes...\n";
      }
      hash->process (&buf[0], count);
      if (cin.fail() && !cin.eof())
      {
         cerr << "Error reading from stream! Exiting!\n";
         break;
      }
   }

   const size_t bufsize = hash->getHashSize();
   boost::scoped_array<char> result (new char[bufsize]);
   size_t hashsize = hash->getHash (result.get(), bufsize);
   dumpHex (result.get(), hashsize);
}

int main (int argc, char ** argv)
{
   std::string name;
   int pos = -1;
   std::auto_ptr<HashFunc> func;

   // Declare the supported options.
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("list", "List registered hash functions")
      ("hash", po::value<std::string>(), "Name of the hash function")
      ("pos", po::value<size_t>(), "Number of the hash function (default 0)")
      ("verbose", "Verbose operation")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if (vm.count("help")) 
   {
      cout << desc << "\n";
      return 1;
   }

   opt_verbose =  (vm.count("verbose"));

   if (vm.count("list"))
   {
      doList ();
      return 0;
   }

   if (vm.count("hash"))
   {
      name = vm["hash"].as<std::string>();
   }
   else if (vm.count("pos"))
   {
      pos = vm["pos"].as<size_t>();
      if (pos == 0 || (size_t) pos > HashFactory::instance().size())
      {
         cerr << format("Invalid hash function number! Number should be in"
               " range [%u,%u]") % 1 % HashFactory::instance().size() << endl;
         return 1;
      }
   }

   if (pos > 0)
   {
      func.reset (HashFactory::instance().getHash (pos-1));
   }
   else if (name.size())
   {
      func.reset (HashFactory::instance().getHash(name));
   }
   else
   {
      cerr << "No hash function specified! Use --pos or --hash" << endl;
      return 1;
   }

   processStdIn (func.get());
}
