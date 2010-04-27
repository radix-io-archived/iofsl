#ifndef IOFWDUTIL_CONFIGURABLE_HH
#define IOFWDUTIL_CONFIGURABLE_HH

#include <typeinfo>

namespace iofwdutil
{

   // Forward
   class ConfigFile;

   /**
    * This class implements an interface for objects that can be 'configured'
    * (through the configuration file).
    *
    */
   struct Configurable
   {
      virtual void configure (const ConfigFile & config) = 0;

      template <typename T>
      inline static void configure_if_needed (T * cfg, 
            const ConfigFile & config);

      virtual ~Configurable () {};
   };

   template <typename T>
   void Configurable::configure_if_needed (T * cfg, const ConfigFile & config)
   {
      // See if the object implements the interface;
      try
      {
         dynamic_cast<Configurable *>(cfg)->configure (config);
      }
      catch (std::bad_cast & bd)
      {
         // It doens't: no need to configure
      }
   }

}

#endif
