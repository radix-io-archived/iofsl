#include "ConfigFile.hh"
#include <boost/format.hpp>

namespace iofwdutil
{

ConfigFile::ConfigFile (ConfigHandle handle)
   : configfile_ (new ConfigContainer (handle))
{
}

ConfigFile::ConfigFile (const ConfigFile & parent, SectionHandle h)
   : configfile_ (parent.configfile_), 
     configsection_ (new ConfigContainer (configfile_->getConfigFile(), h))
{
}

ConfigFile::~ConfigFile ()
{
}

void ConfigFile::error (const std::string & strs) const
{
   // TODO: decent exception later

   // Make a copy because strs might disappear because of stack unwinding
   throw std::string(strs);
}

ConfigFile ConfigFile::openSection (const char * name)
{
   SectionHandle newhandle;

   if (cf_openSection (configfile_->getConfigFile(), 
            configsection_->getSection (),
            name, &newhandle) < 0)
   {
      error (str(boost::format("Error opening section %s!") % name)); 
   }
   return ConfigFile (*this, newhandle);
}


std::string ConfigFile::getKey (const char * name) const
{
   int keysize;

   // first try to get the size
   keysize = cf_getKey (configfile_->getConfigFile (), 
         configsection_->getSection (), name, 0, 0);

   if (keysize < 0)
      error (str(boost::format("Error in getkey '%s'") % name));

   if (keysize == 0)
      return std::string ();

   std::vector<char> buf (keysize + 1);
   keysize = cf_getKey (configfile_->getConfigFile (), 
         configsection_->getSection (), name, &buf[0], buf.size());
   ALWAYS_ASSERT(keysize > 0);

   return std::string (&buf[0]);
   
}

std::string ConfigFile::getKeyDefault (const char * name, const std::string & def) const
{
   try 
   {
      return getKey (name);
   }
   catch (...)
   {
      return def;
   }
}

std::vector<std::string> ConfigFile::getMultiKey (const char * name) const
{
   char ** data;
   size_t count;

   if (cf_getMultiKey (
            configfile_->getConfigFile (), 
            configsection_->getSection (),
            name, &data, &count
          ) < 0)
   {
      error (str(boost::format("Error in cf_getMultiKey '%s'") % name ));
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
