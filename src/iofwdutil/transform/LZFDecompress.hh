#ifndef IOFWDUTIL_TRANSFORM_LZFDECOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_LZFDECOMPRESS_HH

#include "BlockingDecompress.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZFDecompress : public BlockingDecompress
      {

         protected:

            virtual size_t doDecompress (const void * in, size_t insize,
                  void * out, size_t outsize);
      };

      //=====================================================================
   }
}

#endif
