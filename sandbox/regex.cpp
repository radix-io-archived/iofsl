#include <iostream>
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

int main (int argc, char ** args)
{
   if (argc != 2)
   {
      cerr << "Need one argument!\n"; 
      return 1; 
   }
   match_results<const char*> res;
   regex e ("\\s*([^:]*?)\\s*(?::\\s*(\\S+))?\\s*", regex::perl|regex::icase);

   if (regex_match (args[1], res, e))
   {
      cout << "Match: ";
      for (unsigned int i=0; i<res.size(); ++i)
      {
         cout << i << ": " << res[i] << endl;
      }

   }
   else
   {
      cout << "No match!"; 
   }
   return 0; 
}
