#include <boost/format.hpp>
#include <iostream>
#include "iofwdutil/zlog/ZLog.hh"

using namespace std;
using namespace iofwdutil::zlog; 
using namespace boost; 

int main (int argc, char ** args)
{
   if (argc < 3)
   {
      cerr << "Need at least two  arguments!\n"; 
      return 1; 
   }
   cout << str(format("Filtered '%s' -> '%s'\n") % args[1] % ZLog::filterRegEx(args[1])); 
   for (unsigned int i=2; i<static_cast<unsigned int>(argc); ++i)
   {
      cout << str(format("'%s' matches '%s'? ") % args[1] % args[i]) << 
         (ZLog::matchSourceName (args[1], args[i]) ? "yes" : "no") << endl;
   }
   return 0; 
}
