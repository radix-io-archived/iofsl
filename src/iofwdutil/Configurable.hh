#ifndef IOFWDUTIL_CONFIGURABLE_HH
#define IOFWDUTIL_CONFIGURABLE_HH

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

      static void configure_if_needed (Configurable * cfg, 
            const ConfigFile & config);

      virtual ~Configurable () {};
   };

}

#endif
