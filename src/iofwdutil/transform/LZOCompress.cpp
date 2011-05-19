#include "LZOCompress.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"

#include <lzo/lzo1x.h>

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTEncode,
      iofwdutil::transform::LZOCompress,
      "LZO",
      lzoencode);

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      LZOCompress::LZOCompress ()
         : extra_ (false),
           mem_ (new char[extra_ ?
                 LZO1X_999_MEM_COMPRESS :
                 LZO1X_1_MEM_COMPRESS]),
           curtmpsize_ (0)
      {
      }

      size_t LZOCompress::doCompress (const void * in, size_t insize,
            void * out, size_t outsize)
      {
         const size_t reqspace = requiredSpace (insize);
         unsigned char * dst;

         if (reqspace <= outsize)
         {
            dst = static_cast<unsigned char *>(out);
         }
         else
         {
            if (curtmpsize_ < reqspace)
            {
               tmpmem_.reset (new unsigned char[reqspace]);
               curtmpsize_ = reqspace;
            }

            dst = tmpmem_.get ();
         }

         // LZO compress writes up to requiredSpace(inputsize) bytes.

         lzo_uint outs;
         int ret;
         if (extra_)
            ret = lzo1x_999_compress ((const unsigned char *)in, insize,
                  dst, &outs, mem_.get());
         else
            ret = lzo1x_1_compress ((const unsigned char *)in, insize,
                  dst, &outs, mem_.get());

         check (ret);

         // Check for incompressible block
         if (outs >= insize)
            return 0;

         if (dst != out)
            memcpy (out, dst, outs);

         return outs;
      }

      //=====================================================================
   }
}
