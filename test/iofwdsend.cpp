#include <iostream>
#include "iofwdutil/tools.hh"
#include "client/IOFWDClient.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/ZException.hh"

using namespace std;
using namespace client;
using namespace iofwdutil::bmi; 

inline int checkcall (int ret)
{
   if (ret != ZFS_OK)
      cerr << "Call failed: return code: " << ret << endl;
   else 
      cout << "Return code: " << ret << endl; 

   return ret; 
}

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

   zoidfs_handle_t handle; 

   for (unsigned int i=0; i<1000; ++i)
   {
   cout << "Calling zoidfs_null" << endl; 
   checkcall (client.zoidfs_null ());
   }
   cout << "Calling zoidfs_lookup" << endl; 
   checkcall (client.zoidfs_lookup (0, 0, "/", &handle)); 
   cout << "Calling zoidfs_commit" << endl; 
   checkcall (client.zoidfs_commit (&handle)); 
   client.zoidfs_finalize (); 

   return 0; 
}
catch (const iofwdutil::ZException & e)
{
   cerr << "Exception: \n"; 
   cerr << e.toString (); 
}

