#include <boost/format.hpp>
#include <boost/utility.hpp>

#include "c-util/configstoreadapter.h"
#include "ConfigFile.hh"
#include "ConfigException.hh"

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
      ZTHROW (ConfigException() << zexception_msg(str(boost::format (
                  "Error dumping config file: '%s'") % err)));
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
   ZTHROW (ConfigException () << zexception_msg(strs));
}

boost::optional<ConfigFile> ConfigFile::openSectionOptional (const char *
      name) const
{
   SectionHandle newhandle;

   if (cf_openSection (getConfigHandle(), 
            getSectionHandle(),
            name, &newhandle) < 0)
   {
      return boost::optional<ConfigFile> ();
   }
   return ConfigFile (*this, newhandle);
}

ConfigFile ConfigFile::openSectionDefault (const char * name, const ConfigFile & def) const
{
   boost::optional<ConfigFile> ret (openSectionOptional (name));
   if (ret)
      return *ret;
   else
      return def;
}

ConfigFile ConfigFile::openSection (const char * name) const
{
   boost::optional<ConfigFile> ret (openSectionOptional (name));
   if (ret)
      return *ret;
   else
      ZTHROW (CFKeyMissingException () << cfexception_key_name(name));
}

boost::optional<std::string> ConfigFile::getKeyOptional (const char * name)
   const
{
   int keysize;

   // first try to get the size
   keysize = cf_getKey (getConfigHandle (), 
         getSectionHandle(), name, 0, 0);

   if (keysize < 0)
      return boost::optional<std::string>();

   if (keysize == 0)
      return std::string ();

   std::vector<char> buf (keysize + 1);
   keysize = cf_getKey (getConfigHandle (), 
         getSectionHandle(), name, &buf[0], buf.size());
   ALWAYS_ASSERT(keysize > 0);

   return std::string (&buf[0]);
 }

std::string ConfigFile::getKey (const char * name) const
{
   boost::optional<std::string> ret (getKeyOptional (name));
   if (!ret)
      ZTHROW (CFKeyMissingException () << cfexception_key_name(name));
   else
      return *ret;
}

std::string ConfigFile::getKeyDefault (const char * name, const std::string & def) const
{
   boost::optional<std::string> ret (getKeyOptional (name));
   if (!ret)
      return def;
   else
      return *ret;
}

bool ConfigFile::tryGetMultiKey (const char * name, std::vector<std::string> &
      v) const
{
   try
   {
      v = getMultiKey (name);
   }
   catch (CFKeyMissingException & e)
   {
      return false;
   }
   return true;
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
      ZTHROW (CFKeyMissingException () << cfexception_key_name(name));
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

bool ConfigFile::hasMultiKey (const char * name) const
{
   try
   {
      std::vector<std::string> s = getMultiKey (name);
      return true;
   }
   catch (CFKeyMissingException & e)
   {
      return false;
   }
}

bool ConfigFile::hasKey (const char * name) const
{
   int ret = cf_getKey (getConfigHandle (),
         getSectionHandle (),
         name, 0, 0);
   if (ret < 0)
      return false;
   return true;
}

}
