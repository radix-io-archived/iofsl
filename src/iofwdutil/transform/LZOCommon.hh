#ifndef IOFWDUTIL_TRANSFORM_LZOCOMMON_HH
#define IOFWDUTIL_TRANSFORM_LZOCOMMON_HH

#include "LZOException.hh"
#include <lzo/lzo1x.h>

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      /**
       * Common constants, defs, ... for LZO compression
       */
      struct LZOCommon
      {
         LZOCommon ();

         static std::string errorString (int ret);

         static inline int check (int ret);

      };

      //=====================================================================

      int LZOCommon::check (int ret)
      {
         if (ret == LZO_E_OK)
            return ret;

         ZTHROW (LZOException () << lzo_error_code (ret));
      }

      //=====================================================================
   }
}

#endif
