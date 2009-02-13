#include "IOFWDFrontend.hh"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream> 
#include <unistd.h>
#include "iofwdutil/bmi/BMI.hh"
#include <csignal>

namespace iofwd
{
   namespace frontend
   {

//===========================================================================

// Anonymous namespace 
namespace 
{
//===========================================================================

class IOFW
{
public:
   IOFW (IOFWDFrontend & fe);

   void run ();

   // Called when we should stop...
   void destroy ();

   ~IOFW (); 

protected:
   enum { 
      BATCH_SIZE = 64, // The number of unexpected requests we accept
      MAX_IDLE   = 10000 // How long to wait for a request
   }; 

   void handleIncoming (int count, const BMI_unexpected_info * info); 

protected:
   IOFWDFrontend & fe_; 
   iofwdutil::bmi::BMI & bmi_; 

private:
   // We put the structure here not to burden the stack
   BMI_unexpected_info info_[BATCH_SIZE]; 

   volatile sig_atomic_t stop_; 
};


IOFW::IOFW (IOFWDFrontend & fe)
   : fe_ (fe), bmi_ (iofwdutil::bmi::BMI::get()),
   stop_ (false)
{
}

IOFW::~IOFW ()
{
}

void IOFW::handleIncoming (int count, const BMI_unexpected_info  * )
{
}

void IOFW::run ()
{

   std::cout << "IOFW thread running" << std::endl; 

   // Wait 
   while (!stop_)
   {
      const int count = bmi_.testUnexpected (BATCH_SIZE, &info_[0],MAX_IDLE);
      if (count)
         handleIncoming (count, &info_[0]); 
   }
}

void IOFW::destroy ()
{
   stop_ = true; 
}

//===========================================================================
}


//===========================================================================
//===========================================================================
//===========================================================================

IOFWDFrontend::IOFWDFrontend ()
{
}

IOFWDFrontend::~IOFWDFrontend ()
{
}

void IOFWDFrontend::init ()
{
   IOFW * o = new IOFW (*this); 
   implthread_.reset (new boost::thread(boost::bind (&IOFW::run , o))); 
   impl_ = o; 
}

void IOFWDFrontend::destroy ()
{
   static_cast<IOFW *> (impl_)->destroy (); 
   implthread_->join (); 
   implthread_.reset (); 
   delete (static_cast<IOFW*> (impl_)); 

}

//===========================================================================
   }
}
