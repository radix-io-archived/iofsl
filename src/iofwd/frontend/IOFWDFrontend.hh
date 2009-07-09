#ifndef IOFWD_FRONTEND_IOFWDFRONTEND_HH
#define IOFWD_FRONTEND_IOFWDFRONTEND_HH

#include "Frontend.hh"
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/completion/BMIResource.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDFrontend : public Frontend
{
public:

   /// res = the BMI resource to use
   IOFWDFrontend (iofwdutil::completion::BMIResource & res); 

   virtual ~IOFWDFrontend ();

   virtual void init ();

   virtual void run ();

   virtual void destroy (); 

protected:
   void * impl_; 
   boost::scoped_ptr<boost::thread> implthread_; 
   iofwdutil::IOFWDLogSource & log_; 

   iofwdutil::completion::BMIResource & bmires_; 
};

//===========================================================================
   }
}


#endif
