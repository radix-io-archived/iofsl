#ifndef IOFWD_CONFIG_HH
#define IOFWD_CONFIG_HH

#include "iofwd/service/Service.hh"
#include "iofwdutil/ConfigFile.hh"

namespace iofwd
{
   //========================================================================

   class Config : public service::Service
   {
      public:
         Config (service::ServiceManager & man);

         const iofwdutil::ConfigFile & getConfig () const;

      protected:
         iofwdutil::ConfigFile loadConfig (const std::string & file);

      protected:
         iofwdutil::ConfigFile config_;
   };

   //========================================================================
}

#endif
