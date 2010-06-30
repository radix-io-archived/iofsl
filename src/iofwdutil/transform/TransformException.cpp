#include "TransformException.hh"

namespace iofwdutil
{
   namespace iofwdtransform
   {
      //=====================================================================

      TransformException::TransformException (const std::string & msg)
         : ZException (msg)
      {
      }

      //=====================================================================
   }
}
