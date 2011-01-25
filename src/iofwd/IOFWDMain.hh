#ifndef IOFWD_IOFWDMAIN_HH
#define IOFWD_IOFWDMAIN_HH

#include <memory>

#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/ExtraService.hh"

namespace iofwd
{
//===========================================================================

// Forwards
class RequestHandler;
class Config;
class Log;

/**
 * Main object; Represents the whole I/O forwarding server
 * Groups resources and provides a single startup/shutdown point
 *
 * @TODO: Make this (or add somewhere) a servercontrol service which can be
 * used to shutdown the server? Add a CBException parameter to shutdown, so
 * that when a component thread has an exception we can properly shutdown and
 * then rethrow the exception in the main thread.
 */
class IOFWDMain : public service::Service
{
public:

   IOFWDMain (service::ServiceManager & man);

   // Called to initialize the server
   void boot ();

   void run ();

   // Called (possibly from other thread) when the server needs to shut down
   void shutdown ();

protected:

   /// Load custom services
   void loadServices ();

   /// Stop custom services
   void stopServices ();

protected:

   // Service dependencies
   boost::shared_ptr<RequestHandler>     requesthandler_;

   boost::shared_ptr<Log>                log_service_;
   boost::shared_ptr<Config>             config_service_;
   boost::shared_ptr<frontend::Frontend> frontend_;
   iofwdutil::zlog::ZLogSource & mainlog_;

   // COnfig file
   const iofwdutil::ConfigFile config_;

   // If set, don't catch CTRL-C and don't protect threads from signals
   bool notrap_;

   typedef boost::shared_ptr<ExtraService> ServicePtr;
   std::vector<ServicePtr> custom_services_;
};

//===========================================================================
}



#endif
