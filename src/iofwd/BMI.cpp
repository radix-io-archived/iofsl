#include "BMI.hh"
#include "Config.hh"
#include "Log.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwd/ConfigException.hh"
#include "iofwdutil/bmi/BMI.hh"
#include <cstdio>
#include <boost/foreach.hpp>

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
/*
   BMI_addr_t BMI::lookupAddr (const std::string & s) const
   {
      BMI_addr_t addr;
      iofwdutil::bmi::BMI::check (BMI_addr_lookup (&addr, s.c_str()));
      return addr;
   } */

   std::string BMI::singleServerMode (const iofwdutil::ConfigFile & bmiconfig)
   {
      ZLOG_DEBUG(log_, "Single-server mode");
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

      serverrank_ = 0;
      // servercount_ = 1;
      servernames_.clear ();
      servernames_.push_back (ion);
      return ion;
   }

   std::string BMI::multiServerMode (const iofwdutil::ConfigFile & bmiconfig)
   {
      ZLOG_DEBUG(log_, "Multi-server mode");

      std::vector<std::string> list =
         bmiconfig.getMultiKey ("serverlist");

      const char * rank = getenv ("ZOIDFS_SERVER_RANK");
      if (!rank)
      {
         ZTHROW (ConfigException ()
               << iofwdutil::zexception_msg ("No ZOIDFS_SERVER_RANK set!"));
      }
      unsigned int numrank;
      try
      {
         numrank = boost::lexical_cast<unsigned int>(rank);
      }
      catch (boost::bad_lexical_cast & b)
      {
         ZLOG_ERROR(log_, "Invalid value in ZOIDFS_SERVER_RANK!");
         ZTHROW (ConfigException ()
               << iofwdutil::zexception_msg("Could not "
                  "parse ZOIDFS_SERVER_RANK!"));
      }
      if (numrank >= list.size())
      {
         ZTHROW (ConfigException ()
               << iofwdutil::zexception_msg(
                  "Invalid server rank specified!"));
      }
      // servercount_ = list.size();
      serverrank_ = numrank;

      servernames_.swap(list);
      return servernames_[serverrank_];
   }

   void BMI::init ()
   {
      ZLOG_DEBUG (log_, "Initializing BMI");


      ConfigFile bmiconfig (config_.openSection ("bmi"));
      std::string clientmode = bmiconfig.getKeyDefault ("clientmode", "");
      std::string ion;
      if (clientmode.empty())
      {
         fprintf(stderr, "%s:%i\n", __func__, __LINE__);
         if (bmiconfig.hasMultiKey ("serverlist"))
         {
            ion = multiServerMode (bmiconfig);
         }
         else
         {
            ion = singleServerMode (bmiconfig);
         }

         // IOFW uses bmi, so we need to supply init params here
         ZLOG_INFO (log_, format("Server listening on %s") % ion);
         iofwdutil::bmi::BMI::setInitServer (ion.c_str());
      }

      // Force instantiation
      iofwdutil::bmi::BMI::instance ();
   
      /*servers_.reserve (servernames_.size());
      BOOST_FOREACH (const std::string & s, servernames_)
      {
         ZLOG_DEBUG(log_, format("Looking up peer '%s'...") % s);
         servers_.push_back (lookupAddr (s));
      } */
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
