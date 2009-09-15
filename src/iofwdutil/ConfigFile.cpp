#include "ConfigFile.hh"
#include <boost/format.hpp>
#include <boost/utility.hpp>
#include "c-util/configstoreadapter.h"

// @TODO:
//  get decent error codes in cf_getKey/... so we can differentiate
//  between errors and missing keys.
//

namespace iofwdutil
{

ConfigHandle ConfigFile::getConfigHandle () const 
{
   return configfile_->getConfigFile ();
}

SectionHandle ConfigFile::getSectionHandle () const
{
   if (!configsection_)
      return ROOT_SECTION;
   return configsection_->getSection ();
}

void ConfigFile::dumpToStdErr () const
{
   char * err = 0;
   if (cf_dump (
            getConfigHandle (),
            getSectionHandle (),
            &err) < 0)
   {
      throw ZException (str(boost::format (
                  "Error dumping config file: '%s'") % err));
   }

}

ConfigFile::ConfigFile (const ConfigFile & other)
   : configfile_ (other.configfile_), 
     configsection_ (other.configsection_)
{
}

ConfigFile & ConfigFile::operator = (const ConfigFile & other)
{
   if (boost::addressof (*this) == boost::addressof(other))
      return *this;

   // Free old configfile
   configfile_.reset (0);
   configsection_.reset (0);

   /* no need to do deep copy, we do not support modifying config file */
   configfile_ = other.configfile_;
   configsection_ = other.configsection_;

   return *this;
} 

ConfigFile::ConfigFile (ConfigHandle handle)
   : configfile_ (new ConfigContainer (handle))
{
}

ConfigFile::ConfigFile ()
{
   configfile_.reset (new ConfigContainer (cfsa_create_empty ()));
}

ConfigFile::ConfigFile (const ConfigFile & parent, SectionHandle h)
   : configfile_ (parent.configfile_), 
     configsection_ (new ConfigContainer (parent.configfile_->getConfigFile(), h))
{
}

ConfigFile::~ConfigFile ()
{
}

void ConfigFile::error (const std::string & strs) const
{
   // TODO: decent exception later

   // Make a copy because strs might disappear because of stack unwinding
   throw strs;
}

ConfigFile ConfigFile::openSectionDefault (const char * name, const ConfigFile & def) const
{
   try
   {
      return openSection (name);
   }
   catch (const CFKeyMissingException & e)
   {
      return def;
   }
}

ConfigFile ConfigFile::openSection (const char * name) const
{
   SectionHandle newhandle;

   if (cf_openSection (getConfigHandle(), 
            getSectionHandle(),
            name, &newhandle) < 0)
   {
      throw CFKeyMissingException (name);
   }
   return ConfigFile (*this, newhandle);
}


std::string ConfigFile::getKey (const char * name) const
{
   int keysize;

   // first try to get the size
   keysize = cf_getKey (getConfigHandle (), 
         getSectionHandle(), name, 0, 0);

   if (keysize < 0)
      throw CFKeyMissingException (name);

   if (keysize == 0)
      return std::string ();

   std::vector<char> buf (keysize + 1);
   keysize = cf_getKey (getConfigHandle (), 
         getSectionHandle(), name, &buf[0], buf.size());
   ALWAYS_ASSERT(keysize > 0);

   return std::string (&buf[0]);
   
}

std::string ConfigFile::getKeyDefault (const char * name, const std::string & def) const
{
   try 
   {
      return getKey (name);
   }
   catch (const CFKeyMissingException & e)
   {
      return def;
   }
}

std::vector<std::string> ConfigFile::getMultiKey (const char * name) const
{
   char ** data;
   size_t count;

   if (cf_getMultiKey (
            getConfigHandle (), 
            getSectionHandle(),
            name, &data, &count
          ) < 0)
   {
      throw CFKeyMissingException (name);
   }

   std::vector<std::string> ret (count);
   for (size_t i=0; i<count; ++i)
   {
      ret[i] = data[i];
      free (data[i]);
   }
   free (data);
   return ret;
}

}
