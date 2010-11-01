#ifndef IOFWDUTIL_TRANSFORM_BZCOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_BZCOMPRESS_HH

#include "BZLib.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class BZCompress : public BZLib
      {
         public:
         BZCompress ();

      };

      //=====================================================================
   }
}

#endif
