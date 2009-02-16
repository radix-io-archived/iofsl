#include <iostream>
#include "iofwdutil/tools.hh"
#include "client/IOFWDClient.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/ZException.hh"

using namespace std;
using namespace client;
using namespace iofwdutil::bmi; 

int main (int argc, char ** args)
try
{
   if (argc != 2)
   {
      cerr << "Need one argument!\n";
      exit (1); 
   }

   BMI::setInitClient (); 

   // Force initialization 
   BMI::get(); 

   // client opens its own context
   IOFWDClient client (args[1]); 
  
   client.zoidfs_init (); 

   client.zoidfs_null (); 

   client.zoidfs_finalize (); 

   return 0; 
}
catch (const iofwdutil::ZException & e)
{
   cerr << "Exception: \n"; 
   cerr << e.toString (); 
}

