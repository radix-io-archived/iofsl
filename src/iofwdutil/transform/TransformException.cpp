#include "TransformException.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      TransformException::TransformException (const std::string & msg)
         : ZException (msg)
      {
      }

      //=====================================================================
   }
}
