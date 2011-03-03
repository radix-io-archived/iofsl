#include "FTBService.hh"
#include "FTBException.hh"

#include "iofwd/service/ServiceManager.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwd/Log.hh"

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <ftb.h>


SERVICE_REGISTER(iofwd::FTBService, ftb);

using boost::format;

using namespace iofwd::service;

namespace iofwd
{
   //========================================================================
   
   static const char * FTB_EVENT_SPACE = "FTB.IOFSL.IOFSL";

   static FTB_event_info_t ftb_events [] = {
      { "TEST", "INFO" }
   };

   //========================================================================

   FTBService::FTBService (ServiceManager & m)
      : ExtraService (m),
        log_service_ (lookupService<Log>("log")),
        log_ (log_service_->getSource ("ftb")),
        shutdown_ (false)
   {
      ZLOG_INFO (log_, "FTB service starting...");
   }

   void FTBService::configureNested (const iofwdutil::ConfigFile & f)
   {
      opt_sleeptime_ = f.getKeyAsDefault<size_t> ("period", 600);
      ZLOG_DEBUG (log_, format("Will publish every %u seconds...")
            % opt_sleeptime_);
      startFTB ();
   }

   FTBService::~FTBService ()
   {
      ZLOG_INFO (log_, "FTB service shutting down...");
      shutdownFTB ();
   }

   void FTBService::startFTB ()
   {
      thread_.reset (
            new boost::thread (boost::bind (&FTBService::FTBMain, this)));
   }

   int FTBService::checkFTB (int ret) const
   {
      if (ret == FTB_SUCCESS)
         return ret;
      ZLOG_ERROR (log_, format("FTB error message: %s")
            % getFTBErrorString (ret));
      ZTHROW (FTBError () << ftb_errorcode (ret));
   }

   void FTBService::FTBLoop ()
   {
      boost::system_time nextwakeup = boost::get_system_time ();

      while (!needShutdown ())
      {
         FTB_event_handle_t ehandle;

         ZLOG_DEBUG (log_, "Publishing events...");
         checkFTB (FTB_Publish (*ftbhandle_, "TEST", 0, &ehandle));

         nextwakeup += boost::posix_time::seconds (opt_sleeptime_);
         sleep (nextwakeup);
      }
   }

   /**
    * NOTE: This function must be exception safe and properly catch
    * exceptions.
    *
    * @TODO: what to do when FTB fails?
    */
   void FTBService::FTBMain ()
   {
      FTB_client_t clientinfo;

      /* Specify the client information and call FTB_Connect */
      memset(&clientinfo, 0, sizeof(clientinfo));
      strncpy(clientinfo.event_space, FTB_EVENT_SPACE,
            sizeof (clientinfo.event_space));
      // TODO: fix this, unsafe is string gets truncated
      strncpy(clientinfo.client_subscription_style, "FTB_SUBSCRIPTION_NONE",
            sizeof(clientinfo.client_subscription_style));

      while (!needShutdown ())
      {
         try
         {
            ZLOG_DEBUG (log_, "Connection to FTB...");
            FTB_client_handle_t h;
            checkFTB (FTB_Connect (&clientinfo, &h));
            ftbhandle_ = h;
            break;
         }
         catch (FTBError & e)
         {
            ZLOG_ERROR (log_, format("FTB_Connect failed (%s)... Will retry"
                     " in %u seconds...") % getFTBErrorString (e)
                  % opt_sleeptime_);

            sleep (boost::get_system_time () +
                  boost::posix_time::seconds (opt_sleeptime_));
         }
      }

      try
      {
         if (!needShutdown ())
         {
            ZLOG_DEBUG (log_, "Registering events...");
            checkFTB (FTB_Declare_publishable_events (*ftbhandle_, 0,
                     ftb_events, sizeof(ftb_events)/sizeof(ftb_events[0])));
         }
      }
      catch (const FTBError & e)
      {
         ZLOG_ERROR (log_, format("Error registering publishable events (%)"
                  " - fatal: shutting down FTB thread")
               % getFTBErrorString (e));
         setShutdown ();
      }

      if (!needShutdown ())
      {
         FTBLoop ();
      }

      try
      {
         if (ftbhandle_)
         {
            ZLOG_DEBUG (log_, "Disconnecting from FTB...");
            checkFTB (FTB_Disconnect (*ftbhandle_));
         }
      }
      catch (const FTBError & e)
      {
         ZLOG_ERROR (log_, format("FTB_Disconnect failed (ignored): %s")
               % getFTBErrorString (e));
      }
   }

   bool FTBService::sleep (const boost::system_time & abstime)
   {
      boost::mutex::scoped_lock l(lock_);
      cond_.timed_wait (l, abstime);
      return !shutdown_;
   }

   bool FTBService::needShutdown () const
   {
      boost::mutex::scoped_lock l(lock_);
      return shutdown_;
   }

   void FTBService::wakeFTB ()
   {
      boost::mutex::scoped_lock l(lock_);
      cond_.notify_one ();
   }

   void FTBService::setShutdown ()
   {
      boost::mutex::scoped_lock l(lock_);
      shutdown_ = true;
      cond_.notify_one ();
   }

   void FTBService::shutdownFTB ()
   {
      setShutdown ();
      ZLOG_DEBUG (log_, "Waiting for FTB thread to end...");
      thread_->join ();
      thread_.reset ();
   }

   //========================================================================
}
