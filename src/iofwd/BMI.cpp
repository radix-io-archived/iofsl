#include "BMI.hh"
#include "Config.hh"
#include "Log.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwd/ConfigException.hh"
#include "iofwdutil/bmi/BMI.hh"

SERVICE_REGISTER(iofwd::BMI,bmi);

using namespace iofwdutil;
using namespace iofwdutil::bmi;


namespace iofwd
{
   //========================================================================

   BMI::BMI (service::ServiceManager & m)
      : service::Service (m),
        config_service_ (lookupService<Config>("config")),
        log_service_ (lookupService<Log>("log")),
        config_ (config_service_->getConfig()),
        log_ (log_service_->getSource ("bmiservice"))
   {
      init ();
      resource_.reset (new iofwdevent::BMIResource ());
      resource_->start ();
   }

   BMI::~BMI ()
   {
      done ();
   }

   void BMI::init ()
   {
      ZLOG_DEBUG (log_, "Initializing BMI");

      ConfigFile bmiconfig (config_.openSection ("bmi"));
      std::string ion = bmiconfig.getKeyDefault ("listen", "");
      const char * ion_name = getenv("ZOIDFS_ION_NAME");
      if (ion_name)
         ion = ion_name;

      if (ion.empty())
      {
         ZLOG_ERROR (log_, format("ZOIDFS_ION_NAME is empty"));
         ZTHROW (ConfigException ()
               << zexception_msg ("No server listen address specified"
                  " in config file or ZOIDFS_ION_NAME environment variable!")
               << ce_environment ("ZOIDFS_ION_NAME")
               << ce_key ("listen"));
      }

      // IOFW uses bmi, so we need to supply init params here
      ZLOG_INFO (log_, format("Server listening on %s") % ion);
      iofwdutil::bmi::BMI::setInitServer (ion.c_str());

      listen_ = ion;

      // Force instantiation
      iofwdutil::bmi::BMI::instance ();

      // Make sure we have a context open
      // @TODO: get rid of this context stuff (or better, collect all BMI
      // utils and move them into one location/class)

      // @TODO: Maybe merge iofwdutil::BMI functionality in here

      /* bmictx_ = iofwdutil::bmi::BMI::get().openContext ();
      res_.bmictx_ = bmictx_; */
   }

   void BMI::done ()
   {
      ZLOG_INFO (log_, "Shutting down BMI...");

      resource_->stop ();

      resource_.reset ();

      iofwdutil::bmi::BMI::instance().finalize ();
   }

   //========================================================================
}
