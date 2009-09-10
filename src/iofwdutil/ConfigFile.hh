#ifndef IOFWDUTIL_CONFIGFILE_HH
#define IOFWDUTIL_CONFIGFILE_HH

#include "c-util/configfile.h"
#include "ConfigContainer.hh"
#include <boost/smart_ptr.hpp>
#include <string>
#include <vector>

namespace iofwdutil
{


/**
 * C++ wrapper for resource acquisition/release of ConfigHandle.
 *
 */

class ConfigFile
{
public:
   /**
    * Default copy and assignment operator is fine
    */

   /// Takes ownership of ConfigHandle 
   ConfigFile (ConfigHandle handle);

   /// Return a new ConfigFile object for the specified subsection
   ConfigFile openSection (const char * name);

   /// Return key value, exception on error (such as missing)
   std::string getKey (const char * name) const;

   /// Return key value or default if key is missing
   std::string getKeyDefault (const char * name, const std::string & def) const;

   /// Return multikey array or exception if missing
   std::vector<std::string> getMultiKey (const char * name) const;

   ~ConfigFile ();

protected:
   ConfigFile (const ConfigFile & parent, SectionHandle h);

   // Called to throw exception
   void error (const std::string & s) const;

protected:
   boost::intrusive_ptr<ConfigContainer> configfile_;
   boost::intrusive_ptr<ConfigContainer> configsection_;
};

}
#endif
