#ifndef IOFWDUTIL_TRANSFORM_LZFCOMMON_HH
#define IOFWDUTIL_TRANSFORM_LZFCOMMON_HH

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      /**
       * Common constants, defs, ... for LZF compression
       */
      struct LZFCommon
      {
         // @TODO: get this info from common header with C lzf stream code
            enum { LZF_BLOCK_SIZE = 64000,  // maximum size of block in stream
                   LZF_HEADER_SIZE = 5,     // Size of block header
            
                   HEADER_BLOCK_COMPRESSED = 'C',
                   HEADER_BLOCK_UNCOMPRESSED = 'U'
            };

      };

      //=====================================================================
   }
}

#endif
