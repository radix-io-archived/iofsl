#include "FTBService.hh"
#include "FTBException.hh"

#include "iofwd/service/ServiceManager.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwd/Log.hh"
#include "iofwd/BMI.hh"

// #include "common/ftb/ftb.pb.h"

#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <arpa/inet.h>

#include <ftb.h>


SERVICE_REGISTER(iofwd::FTBService, ftb);

using boost::format;

using namespace iofwd::service;

namespace iofwd
{
   //========================================================================
   
   static const char * FTB_EVENT_SPACE = "IOFSL.IOFSL.IOFSL";

   static FTB_event_info_t ftb_events [] = {
      { "loadupdate", "INFO" }
   };

   //========================================================================

   FTBService::FTBService (ServiceManager & m)
      : ExtraService (m),
        log_service_ (lookupService<Log>("log")),
        bmi_service_ (lookupService<BMI>("bmi")),
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

   void FTBService::publishPB (const char * name,
         const google::protobuf::MessageLite & msg)
   {
      std::string out;
      // if (!msg.SerializeToString (&out))
      //   return;

      FTB_event_properties prop;

      char * dest = (char*) &prop.event_payload;
      *(uint32_t *) dest = htonl (out.size ());
      size_t used = 0;
      dest += sizeof (uint32_t);
      used += sizeof (uint32_t);

      prop.event_type = 1;
      ALWAYS_ASSERT(used <= FTB_MAX_PAYLOAD_DATA);
      memcpy (dest, out.c_str(), out.size ());
      FTB_event_handle_t ehandle;
      checkFTB (FTB_Publish (*ftbhandle_, name, &prop, &ehandle));
   }

   void FTBService::FTBLoop ()
   {
      boost::system_time nextwakeup = boost::get_system_time ();

      // ftb::LoadUpdate load_update;

      // @TODO: get uuid (or more general server identity) in service?
      boost::uuids::random_generator gen;
      boost::uuids::uuid u = gen ();

      ZLOG_INFO (log_, format("Server UUID: %s") % u);

      // load_update.mutable_id()->set_location(bmi_service_->getListenAddress ());
      //load_update.mutable_id()->set_uuid (&u, sizeof (u));

      while (!needShutdown ())
      {
         // FTB_event_handle_t ehandle;

         ZLOG_DEBUG (log_, "Publishing events...");

         // load_update.set_load (0.5);

         // publishPB ("loadupdate", load_update);

         //checkFTB (FTB_Publish (*ftbhandle_, "loadupdate", 0, &ehandle));

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
