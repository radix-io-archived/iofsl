#include "BZDecompress.hh"

#include "iofwdutil/LinkHelper.hh"

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTDecode,
      iofwdutil::transform::BZDecompress,
      "BZLIB",
      bzlibdecode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      BZDecompress::BZDecompress ()
         : BZLib (false)
      {
      }

      //=====================================================================
   }
}
