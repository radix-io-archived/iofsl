#include "IOFWDMain.hh"
#include "DefRequestHandler.hh"
#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/signals.hh"

using namespace iofwdutil; 

namespace iofwd
{
//===========================================================================

IOFWDMain::IOFWDMain (bool notrap, const iofwdutil::ConfigFile & co)
   : mainlog_ (IOFWDLog::getSource ()), notrap_(notrap),
     config_ (co)
{
   // Make sure that we do have signals sent to a random thread
   disableAllSignals (notrap); 
}

void IOFWDMain::boot ()
{
  
   ZLOG_DEBUG (mainlog_, "Starting IOFWD Frontend"); 
   frontend_.reset (new frontend::IOFWDFrontend (bmires_));
   
   frontend_->setConfig (config_.openSectionDefault ("frontend"));

   frontend_->init ();

   // Set handler for frontend
   requesthandler_.reset (new DefRequestHandler ()); 
   frontend_->setHandler (requesthandler_.get());


   // Start frontend and begin accepting requests
   frontend_->run ();
}


void IOFWDMain::shutdown ()
{
   ZLOG_DEBUG (mainlog_, "Stopping IOFWD Frontend"); 
   frontend_->destroy (); 

   requesthandler_.reset (); 
}


void IOFWDMain::run ()
{
   // Wait for ctrl-c
   sigset_t set; 
   sigemptyset (&set); 
   
   if (!notrap_)
      sigaddset (&set, SIGINT); 

   sigaddset (&set, SIGUSR1); 

   waitSignal (&set); 
}


//===========================================================================
}
