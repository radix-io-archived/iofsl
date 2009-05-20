#ifndef IOFWD_FRONTEND_IOFWDFRONTEND_HH
#define IOFWD_FRONTEND_IOFWDFRONTEND_HH

#include "Frontend.hh"
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDFrontend : public Frontend
{
public:
public:
   IOFWDFrontend (); 

   virtual ~IOFWDFrontend ();

   virtual void init (); 

   virtual void destroy (); 

protected:
   void * impl_; 
   boost::scoped_ptr<boost::thread> implthread_; 
};

//===========================================================================
   }
}


#endif
