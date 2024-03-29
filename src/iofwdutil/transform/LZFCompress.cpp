#include "LZFCompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"

extern "C" {
#include "src/c-util/transform/lzf/lzf.h"
}

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTEncode,
      iofwdutil::transform::LZFCompress,
      "LZF",
      lzfencode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      size_t LZFCompress::doCompress (const void * in, size_t insize,
            void * out, size_t outsize)
      {
         return lzf_compress (in, insize, out, outsize);
      }

      //=====================================================================
   }
}
