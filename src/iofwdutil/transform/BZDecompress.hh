#ifndef IOFWDUTIL_TRANSFORM_BZDECOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_BZDECOMPRESS_HH

#include "BZLib.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class BZDecompress : public BZLib
      {
         public:
         BZDecompress ();

      };

      //=====================================================================
   }
}

#endif
