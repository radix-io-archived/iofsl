#include "IOFWDMain.hh"
#include "DefRequestHandler.hh"
#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/signals.hh"
#include "iofwdutil/ConfigException.hh"
#include "service/ServiceException.hh"
#include "iofwdutil/stats/CounterTable.hh"

// Services
#include "Log.hh"
#include "Config.hh"

#include <boost/foreach.hpp>

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

void IOFWDMain::loadServices ()
{
   ZLOG_INFO (mainlog_, "Starting extra services...");
   ConfigFile config (config_.openSectionDefault ("extra_services"));
   try
   {
      std::vector<std::string> keys = config.getMultiKey ("services");
      BOOST_FOREACH (const std::string & s, keys)
      {
         ZLOG_INFO (mainlog_,
             boost::format("Trying to load extra service '%s'...") % s);
         custom_services_.push_back (lookupService<ExtraService> (s));

         // Configure nested service
         custom_services_.back()->configureNested (
               config.openSectionDefault (s.c_str()));
      }
   }
   catch (const CFKeyMissingException & e)
   {
      // It's ok if there's no services key...
   }
   // Provide a user error message
   catch (service::UnknownServiceException & e)
   {
      addUserErrorMessage (e, str(boost::format(
          "Service '%s' unknown. Was the service enabled"
          " at configure time?") %
            *iofwdutil::zexception_info<service::service_name>(e)));
      throw;
   }
}

void IOFWDMain::stopServices ()
{
   ZLOG_INFO (mainlog_, "Stopping extra services...\n");
   custom_services_.clear ();
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

   // Load any extra services
   loadServices ();
}


void IOFWDMain::shutdown ()
{
   stopServices ();

   ZLOG_DEBUG (mainlog_, "Stopping IOFWD Frontend"); 
   frontend_->destroy (); 

   requesthandler_.reset ();

   ZLOG_DEBUG (mainlog_, "Stopping thread pool...");
   iofwdutil::ThreadPool::instance().reset(); 

   ZLOG_DEBUG (mainlog_, "Dumping counters...");
   iofwdutil::stats::CounterTable::instance().dumpCounters();
}


void IOFWDMain::run ()
{
   // Wait for ctrl-c
   sigset_t set; 
   sigemptyset (&set); 
   
   if (!notrap_)
      sigaddset (&set, SIGINT); 

   sigaddset (&set, SIGUSR1); 

   int signal = waitSignal (&set);

   ALWAYS_ASSERT(signal == SIGINT || signal == SIGUSR1);
}


//===========================================================================
}
