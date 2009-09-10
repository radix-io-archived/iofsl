#include "ConfigContainer.hh"

namespace iofwdutil
{



ConfigContainer::ConfigContainer (ConfigHandle h)
   : handle_(h), section_(ROOT_SECTION)
{
}

ConfigContainer::ConfigContainer (ConfigHandle h, SectionHandle section)
   : handle_(h), section_ (section)
{
}

ConfigContainer::~ConfigContainer ()
{
   if (section_ != ROOT_SECTION)
   {
      // We own the section, not the configfile handle
      cf_closeSection (handle_, section_);
   }
   else
   {
      // we don't own a section, so we own the handle
      cf_free (handle_);
   }
}


}
