#ifndef IOFWDUTIL_TRANSFORM_LZOEXCEPTION_HH
#define IOFWDUTIL_TRANSFORM_LZOEXCEPTION_HH

#include "TransformException.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      struct LZOException : virtual public TransformException {};

      /**
       * LZO library error code tag
       */
      typedef boost::error_info<struct tag_lzo_error,int> lzo_error_code;

      std::string to_string (const lzo_error_code & err);

      //=====================================================================
   }
}

#endif
