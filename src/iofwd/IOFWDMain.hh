#ifndef IOFWD_IOFWDMAIN_HH
#define IOFWD_IOFWDMAIN_HH

#include <memory>

#include <boost/scoped_ptr.hpp>

#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/IOFSLKeyValueStorage.hh"
#include "iofwd/service/Service.hh"

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
   boost::scoped_ptr<iofwdutil::IOFSLKeyValueStorage> kvstore_;
};

//===========================================================================
}



#endif
