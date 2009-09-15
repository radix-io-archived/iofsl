#ifndef IOFWDUTIL_CONFIGFILE_HH
#define IOFWDUTIL_CONFIGFILE_HH

#include "c-util/configfile.h"
#include "ConfigContainer.hh"
#include "CFKeyMissingException.hh"
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
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

   /// Takes ownership of ConfigHandle; Should not be modified externally
   ConfigFile (ConfigHandle handle);

   ConfigFile (const ConfigFile & other);

   ConfigFile ();

   ConfigFile & operator = (const ConfigFile & other);

   /// Return a new ConfigFile object for the specified subsection
   ConfigFile openSection (const char * name) const;

   /// Return new configfile for subsection, or def if section is missing
   ConfigFile openSectionDefault (const char * name, const ConfigFile & def =
         ConfigFile ()) const;

   /// Return key value, exception on error (such as missing)
   std::string getKey (const char * name) const;

   /// Return key value or default if key is missing
   std::string getKeyDefault (const char * name, const std::string & def) const;

   /// Return multikey array or exception if missing
   std::vector<std::string> getMultiKey (const char * name) const;

   /// Return key value as certain type. Throws bad_cast if exception fails.
   template <typename T> 
   T getKeyAs (const char * name) const;

   /// Return key value as certain type, or default if missing. Throws
   //bad_cast
   template <typename T>
   T getKeyAsDefault (const char * name, const T & def) const;

   ~ConfigFile ();


   void dumpToStdErr () const;

protected:
   ConfigFile (const ConfigFile & parent, SectionHandle h);

   // Called to throw exception
   void error (const std::string & s) const;

   ConfigHandle getConfigHandle () const;

   SectionHandle getSectionHandle () const;

protected:
   boost::intrusive_ptr<ConfigContainer> configfile_;
   boost::intrusive_ptr<ConfigContainer> configsection_;
};


template <typename T> 
T ConfigFile::getKeyAs (const char * name) const
{
   return boost::lexical_cast<T> (getKey (name));
}

template <typename T>
T ConfigFile::getKeyAsDefault (const char * name, const T & def) const
{
   try
   {
   return boost::lexical_cast<T> (getKey (name));
   }
   catch (const CFKeyMissingException & e)
   {
      return def;
   }
}


}
#endif
