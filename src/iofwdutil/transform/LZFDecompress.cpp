#include "LZFDecompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"

extern "C" {
#include "src/c-util/transform/lzf/lzf.h"
}

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTDecode,
      iofwdutil::transform::LZFDecompress,
      "LZF",
      lzfdecode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      size_t LZFDecompress::doDecompress (const void * in, size_t insize,
            void * out, size_t outsize)
      {
          return lzf_decompress (in, insize, out, outsize);
      }

      //=====================================================================
   }
}
