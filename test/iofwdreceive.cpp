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


/**
 * Print out request info and run request sequentially
 */
class TextHandler : public RequestHandler
{
public:

   // NOTE: this is called from a different thread...
   virtual void handleRequest (int count, Request ** reqs)
   {
      for (int i=0; i<count; ++i)
      {
         cout << "opid " << reqs[i]->getOpID() <<
            " (" << reqs[i]->opid2Name() << ")" << endl;  
         while (reqs[i]->run())
         {
         cout << "Running op..." ;
         cout.flush (); 
         }
         delete (reqs[i]); 
      }
   }

   virtual ~TextHandler ()
   {
   }; 
};

int main (int argc, char ** args)
try
{
   if (argc < 2)
   {
      cerr << "Need one argument: listen address...\n"; 
      return 1; 
   }

   sigset_t sigset; 
   sigfillset (&sigset); 
   sigprocmask (SIG_BLOCK, &sigset, 0); 

   TextHandler handler; 
   IOFWDFrontend f;

   f.setHandler (&handler); 

   // Init BMI
   BMI::setInitServer (args[1]); 

   // Make sure we have a context open
   BMIContextPtr ctx = BMI::get().openContext (); 

   cout << "Initializing frontend..." << endl; 
   f.init (); 
  
   int sig; 
   sigfillset (&sigset); 
   sigaddset (&sigset, SIGTERM); 
   sigwait (&sigset, &sig); 
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

