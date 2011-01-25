#ifndef IOFWD_NESTEDCONFIGURABLE_HH
#define IOFWD_NESTEDCONFIGURABLE_HH

#include "iofwdutil/ConfigFile.hh"

namespace iofwd
{
   //========================================================================

   struct NestedConfigurable
   {
      /**
       * This function gets called to indicate where the nested configurable
       * should look for its configuration settings.
       */
      virtual void configureNested (const iofwdutil::ConfigFile & nested) = 0;

      virtual ~NestedConfigurable ();
   };

   //========================================================================
}

#endif
