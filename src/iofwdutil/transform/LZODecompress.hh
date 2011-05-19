#ifndef IOFWDUTIL_TRANSFORM_LZODECOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_LZODECOMPRESS_HH

#include "BlockingDecompress.hh"
#include "LZOCommon.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZODecompress : public BlockingDecompress, public LZOCommon
      {
         protected:

            virtual size_t doDecompress (const void * in, size_t insize,
                  void * out, size_t outsize);

         private:

            char workmem_;

      };

      //=====================================================================
   }
}
#endif
