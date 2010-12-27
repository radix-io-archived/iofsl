#include "IOFWDMain.hh"
#include "DefRequestHandler.hh"
#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/signals.hh"

// Services
#include "Log.hh"
#include "Config.hh"

using namespace iofwdutil;

SERVICE_REGISTER(iofwd::IOFWDMain, iofwdserver);

namespace iofwd
{
//===========================================================================

IOFWDMain::IOFWDMain (service::ServiceManager & man)
   : service::Service (man),
     log_service_ (lookupService<Log>("log")),
     config_service_ (lookupService<Config>("config")),
     frontend_ (lookupService<frontend::Frontend>("bmifrontend")),
     mainlog_ (log_service_->getSource ()),
     config_ (config_service_->getConfig ()),
     notrap_ (!manager_.getParam ("iofwdserver.notrap").empty())
{
   
   // Make sure that we do have signals sent to a random thread
   disableAllSignals (notrap_);
}

void IOFWDMain::boot ()
{
   ZLOG_DEBUG (mainlog_, "Starting IOFWD Frontend"); 

   //frontend_.reset (new frontend::IOFWDFrontend (*resources_));
   
   frontend_->setConfig (config_.openSectionDefault ("frontend"));

   frontend_->init ();

   // Set handler for frontend
   requesthandler_.reset (new DefRequestHandler(config_.openSectionDefault("requesthandler"))); 
   frontend_->setHandler (requesthandler_.get());

   // Start frontend and begin accepting requests
   frontend_->run ();
}


void IOFWDMain::shutdown ()
{
   ZLOG_DEBUG (mainlog_, "Stopping IOFWD Frontend"); 
   frontend_->destroy (); 

   requesthandler_.reset ();

   ZLOG_DEBUG (mainlog_, "Stopping thread pool...");
   iofwdutil::ThreadPool::instance().reset(); 
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
