#include "Config.hh"

#include "c-util/txt_configfile.h"
#include "iofwd/service/ServiceManager.hh"
#include "iofwd/ConfigException.hh"

#include <boost/format.hpp>

using namespace iofwdutil;

SERVICE_REGISTER(iofwd::Config, config);

namespace iofwd
{
   //========================================================================

   Config::Config (service::ServiceManager & man)
      : service::Service (man)
   {
      std::string file = man.getParam ("config.configfile");
      if (file.empty())
      {
         ZTHROW (ConfigException ()
               << iofwdutil::zexception_msg("config.configfile needs to be"
                  " set!"));
      }
      config_ = loadConfig (file);
   }


   iofwdutil::ConfigFile Config::loadConfig (const std::string & name)
   {
      char * err = 0;
      try
      {
         ConfigHandle h = txtfile_openConfig (name.c_str(), &err);
         if (!h || err)
         {
            const std::string s =
               str(boost::format("Error opening config file '%s': %s")
                     % name
                     % err);
            free (err);
            ZTHROW (ConfigIOException () <<  zexception_msg (s));
         }

         return ConfigFile  (h);
      }
      catch (iofwdutil::ConfigException & e)
      {
         // Add filename to exception
         e << cfexception_file_name (name);
         throw;
      }
   }


   const iofwdutil::ConfigFile & Config::getConfig () const
   {
      return config_;
   }

   //========================================================================
}
