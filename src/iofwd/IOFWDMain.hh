#ifndef IOFWD_IOFWDMAIN_HH
#define IOFWD_IOFWDMAIN_HH

#include <memory>

#include <boost/scoped_ptr.hpp>

#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/zlog/ZLogSource.hh"
#include "iofwdutil/ConfigFile.hh"
#include "Resources.hh"
#include "iofwdutil/IOFSLKeyValueStorage.hh"

namespace iofwd
{
//===========================================================================

// Forwards
class RequestHandler;

/**
 * Main object; Represents the whole I/O forwarding server
 * Groups resources and provides a single startup/shutdown point
 */
class IOFWDMain
{
public:

   IOFWDMain (bool notrap, const iofwdutil::ConfigFile & config_);

   // Called to initialize the server
   void boot ();

   void run ();

   // Called (possibly from other thread) when the server needs to shut down
   void shutdown ();

protected:
   std::auto_ptr<frontend::Frontend> frontend_;
   std::auto_ptr<RequestHandler> requesthandler_;

   iofwdutil::zlog::ZLogSource & mainlog_;

   // ===== Resources ====
   std::auto_ptr<Resources> resources_;

   // If set, don't catch CTRL-C and don't protect threads from signals
   bool notrap_;

   // COnfig file
   const iofwdutil::ConfigFile & config_;

   boost::scoped_ptr<iofwdutil::IOFSLKeyValueStorage> kvstore_;
};

//===========================================================================
}



#endif
