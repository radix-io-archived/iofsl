#include "iofwd/frontend/IOFWDFrontend.hh"
#include <iostream>
#include <signal.h>
#include "iofwd/RequestHandler.hh"
#include "iofwd/Request.hh"
#include "iofwdutil/tools.hh"
#include <unistd.h>
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/ZException.hh"

using namespace std; 
using namespace iofwd;
using namespace iofwd::frontend;
using namespace iofwdutil::bmi; 

class TextHandler : public RequestHandler
{
public:

   // NOTE: this is called from a different thread...
   virtual void handleRequest (int count, Request * reqs)
   {
      for (int i=0; i<count; ++i)
      {
         cout << "opid " << reqs[i].getOpID() << endl;  
      }
   }

   virtual ~TextHandler ()
   {
   }; 
};

int main (int argc, char ** args)
try
{
   if (argc != 2)
   {
      cerr << "Need one argument: listen address...\n"; 
      return 1; 
   }

   IOFWDFrontend f;


   // Init BMI
   BMI::setInitServer (args[1]); 

   cout << "Initializing frontend..." << endl; 
   f.init (); 
  
   sleep (6); 
   cout << "Shutting down frontend..." << endl; 
   f.destroy (); 
   cout << "Frontend shut down..." << endl; 

   return 0; 
}
catch (const iofwdutil::ZException & e)
{
   cerr << "Exception: \n"; 
   cerr << e.toString (); 
}

