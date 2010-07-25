#include "BZCompress.hh"

#include "iofwdutil/LinkHelper.hh"

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTEncode,
      iofwdutil::transform::BZCompress,
      "BZLIB",
      bzlibencode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      BZCompress::BZCompress ()
         : BZLib (true)
      {
      }

      //=====================================================================
   }
}
