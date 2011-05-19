#ifndef IOFWDUTIL_ZOIDFS_LZOCOMPRESS_HH
#define IOFWDUTIL_ZOIDFS_LZOCOMPRESS_HH

#include "BlockingCompress.hh"
#include "LZOCommon.hh"

#include <boost/scoped_array.hpp>

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZOCompress : public BlockingCompress, public LZOCommon
      {
         public:
            LZOCompress ();

         protected:
            size_t requiredSpace (size_t input)
            { return input + input/16 + 64 + 3; }

         protected:
            
            virtual size_t doCompress (const void * in, size_t insize,
                  void * out, size_t outsize);

         private:
            bool                      extra_;
            boost::scoped_array<char> mem_;
            boost::scoped_array<unsigned char> tmpmem_;
            size_t curtmpsize_;
      };

      //=====================================================================
   }
}
#endif
