#include "LZOException.hh"
#include "LZOCommon.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      std::string to_string (const lzo_error_code & err)
      {
         return "LZO library error: " + LZOCommon::errorString (err.value ());
      }

      //=====================================================================
   }
}
