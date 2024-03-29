#ifndef IOFWDUTIL_TRANSFORM_LZFCOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_LZFCOMPRESS_HH

#include "BlockingCompress.hh"


namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZFCompress : public BlockingCompress
      {
         protected:

            virtual size_t doCompress (const void * in, size_t insize,
                  void * out, size_t outsize);

      };

      //=====================================================================
   }
}
#endif
