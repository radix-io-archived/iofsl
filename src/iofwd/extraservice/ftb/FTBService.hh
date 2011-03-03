#ifndef IOFWD_FTBSERVICE_HH
#define IOFWD_FTBSERVICE_HH

#include "iofwd/ExtraService.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

#include <ftb.h>
#include <boost/thread.hpp>
#include <boost/optional.hpp>

namespace iofwd
{
   //========================================================================

   class Log;

   /**
    * FTB service
    *
    * @TODO: if we need to add another service that gets its own worker
    * thread, get the code from here and put in a base class
    */
   class FTBService : public ExtraService
   {
      public:
         FTBService (service::ServiceManager & m);

         virtual ~FTBService ();

         virtual void configureNested (const iofwdutil::ConfigFile & f);

      protected:

         /// Start FTB thread
         void startFTB ();

         /// FTB thread
         void FTBMain ();

         /// FTB Main loop
         void FTBLoop ();

         /// (For FTB thread): until specified time, unless we get interrupted
         /// Returns false if we should shutdown
         bool sleep (const boost::system_time & abstime);

         /// Returns true if we should shut down
         bool needShutdown () const;

         /// Signal and wait until FTB shuts down
         void shutdownFTB ();

         /// Indicate shutdown is needed
         void setShutdown ();

         /// Wakeup thread and republish now (if possible)
         void wakeFTB ();

      protected:
         int checkFTB (int ret) const;

      protected:
         boost::shared_ptr<Log> log_service_;

         iofwdutil::IOFWDLogSource & log_;

         boost::scoped_ptr<boost::thread> thread_;

         mutable boost::mutex lock_;
         boost::condition_variable cond_;
         bool shutdown_;

         /// How long we sleep between publishing events
         size_t opt_sleeptime_;

         /// FTB Client handle
         boost::optional<FTB_client_handle_t> ftbhandle_;
   };

   //========================================================================
}

#endif
