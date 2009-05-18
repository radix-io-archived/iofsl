#ifndef IOFWDUTIL_ZCONFIG_CONFIGPARSER_HH
#define IOFWDUTIL_ZCONFIG_CONFIGPARSER_HH

#include <iostream>
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


protected:

}; 

//===========================================================================
   }
}


#endif
