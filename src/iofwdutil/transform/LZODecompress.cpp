#include "LZODecompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"

#include <lzo/lzo1x.h>

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTDecode,
      iofwdutil::transform::LZODecompress,
      "LZO",
      lzodecode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      size_t LZODecompress::doDecompress (const void * in, size_t insize,
            void * out, size_t outsize)
      {
         size_t outs = outsize;
         const int ret = lzo1x_decompress ((const unsigned char *) in, insize,
               (unsigned char *) out, &outs, &workmem_);
         ALWAYS_ASSERT(ret == LZO_E_OK);
         return outs;
      }

      //=====================================================================
   }
}
