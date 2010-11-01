#include "HashException.hh"

namespace iofwd
{
   namespace frontend
   {
      //=====================================================================

      HashException::HashException (const std::string & msg)
         : iofwdutil::ZException (msg)
      {
      }

      //=====================================================================
   }
}
