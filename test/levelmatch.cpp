#include <iostream>
#include <vector>

#include "iofwdutil/zlog/LevelMatch.hh"
#include "iofwdutil/zlog/ZLogException.hh"
#include "iofwdutil/assert.hh"

using namespace iofwdutil::zlog;
using namespace std; 



bool parseStr (const char * str, std::vector<bool> & vec)
{
   vec.resize (MAX_LEVEL, false); 

   
   try
   {
      LevelMatch m (str); 

      for (unsigned int i=0; i<vec.size(); ++i)
         vec[i] = m(i); 
   }
   catch (ZLogException & e)
   {
      cerr << "Exception: " << e.toString () << endl; 
      return false; 
   }
      
   return true; 
}

int main (int argc, char ** args)
{
   std::vector<bool> v; 

   if (argc != 2)
   {
      cerr << "Need regexp!\n";
      return 1; 
   }

   if (!parseStr (args[1], v))
   {
      cerr << "Could not parse '" << args[1] << "'!\n"; 
      return 1; 
   }
   for (unsigned int i=0; i<v.size(); ++i)
   {
      cout << i << ": " << (v[i] ? "active" : "disabled") << endl;
   }
   return 0;
}
