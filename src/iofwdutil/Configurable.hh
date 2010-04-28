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
      Configurable * ptr = 0;
      try
      {
         ptr = dynamic_cast<Configurable *>(cfg);
      }
      catch (std::bad_cast & bd)
      {
         // It doens't: no need to configure
         return;
      }
      // NOTE: it is important that this step is *outside* of the try block;
      // Otherwise, if anything in the configure method throws a bad_cast, we
      // will hide the exception and continue as if nothing happened.
      //
      // Note that bad_lexical_cast derives from bad_cast, so any errors
      // caused by getKeyAs in configure would just be dropped if we'd try to
      // configure within the try block above!
      ptr->configure (config);
   }

}

#endif
