#ifndef IOFWDUTIL_CONFIGCONTAINER_HH
#define IOFWDUTIL_CONFIGCONTAINER_HH

#include "c-util/configfile.h"
#include "IntrusiveHelper.hh"

namespace iofwdutil
{

class ConfigFile;

class ConfigContainer  : public IntrusiveHelper
{
protected:
   friend class ConfigFile;

   /// Used when the container owns the confighandle
   ConfigContainer (ConfigHandle handle);

   /// Only intended to be used by ConfigFile
   /// Used when the container owns the section (and NOT the handle)
   ConfigContainer (ConfigHandle handle, SectionHandle section);

   ConfigHandle getConfigFile ()
   { return handle_; }

   SectionHandle getSection () 
   { return section_; }

public:
   ~ConfigContainer ();

protected:
   ConfigHandle handle_;
   SectionHandle section_;
};


INTRUSIVE_PTR_HELPER(ConfigContainer);

}

#endif
