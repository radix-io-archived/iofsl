#include "ZException.hh"

namespace iofwdutil
{

   struct ConfigFileException : public virtual ZException {};

   struct CFKeyMissingException : public virtual ConfigFileException {};

   typedef boost::error_info<struct tag_cf_key_name,std::string>
      configfile_key_name;

}
