#ifndef IOFWDUTIL_ZCONFIG_CONFIGPARSER_HH
#define IOFWDUTIL_ZCONFIG_CONFIGPARSER_HH

#include <iosfwd>
#include <string>

namespace iofwdutil
{
   namespace zconfig
   {
//===========================================================================

class ConfigParser 
{
public: 
   ConfigParser (std::istream & in);

   ConfigParser (const ConfigParser & other, const std::string & section); 

   ~ConfigParser (); 


   /** Return a configparser for a subsection */ 
   ConfigParser getSubSection (const std::string & section)
   {
      return ConfigParser (*this, section); 
   }


   /** 
    * Return key 
    *
    * Use own value type that has an boost::any and member T as<T> ()
    *
   template <typename T>
   T & get (const std::string & key) const;  
   */
protected:

}; 

//===========================================================================
   }
}


#endif
