#ifndef IOFWDUTIL_CONFIGFILE_HH
#define IOFWDUTIL_CONFIGFILE_HH

#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

#include "c-util/configfile.h"
#include "ConfigContainer.hh"
#include "ConfigException.hh"

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

   /// Try to access multikey; return false if key is missing; throw if error
   /// occured.
   bool tryGetMultiKey (const char * name, std::vector<std::string> & s)
      const;

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

   ConfigHandle getConfigHandle () const;

   SectionHandle getSectionHandle () const;

   /// Return true if named key is present and the key type is single-value
   bool hasKey (const char * name) const;

   /// Return true if a multi-key is present with the given name.
   /// Returns false if it is not a multi key or is not present
   bool hasMultiKey (const char * name) const;

protected:

   template <typename T>
   static inline T our_lexical_cast (const std::string & s);

   ConfigFile (const ConfigFile & parent, SectionHandle h);

   // Called to throw exception
   void error (const std::string & s) const;

   boost::optional<std::string> getKeyOptional (const char * name) const;

   boost::optional<ConfigFile> openSectionOptional (const char * name) const;

protected:
   boost::intrusive_ptr<ConfigContainer> configfile_;
   boost::intrusive_ptr<ConfigContainer> configsection_;
};

template <typename T>
T ConfigFile::our_lexical_cast (const std::string & s)
{
   return boost::lexical_cast<T> (s);
}


template <>
inline bool ConfigFile::our_lexical_cast<bool> (const std::string & s)
{
   if (boost::iequals (s,"true") || boost::iequals(s,"yes"))
      return true;
   if (boost::iequals (s,"false") || boost::iequals(s,"no"))
      return false;

   return boost::lexical_cast<bool> (s);
}

template <typename T>
T ConfigFile::getKeyAs (const char * name) const
{
   return our_lexical_cast<T> (getKey (name));
}


template <typename T>
T ConfigFile::getKeyAsDefault (const char * name, const T & def) const
{
   boost::optional<std::string> ret (getKeyOptional(name));
   if (!ret)
      return def;
   else
      return our_lexical_cast<T> (*ret);
}


}
#endif
