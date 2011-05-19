#ifndef IOFWDUTIL_TRANSFORM_BLOCKINGCOMMON_HH
#define IOFWDUTIL_TRANSFORM_BLOCKINGCOMMON_HH

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      /**
       * Common constants, defs, ... for block based compression
       */
      struct BlockingCommon
      {
         // @TODO: get this info from common header with C lzf stream code
            enum { BLOCK_SIZE = 64000,  // maximum size of block in stream
                   HEADER_SIZE = 5,     // Size of block header
            
                   HEADER_BLOCK_COMPRESSED = 'C',
                   HEADER_BLOCK_UNCOMPRESSED = 'U'
            };

      };

      //=====================================================================
   }
}

#endif
