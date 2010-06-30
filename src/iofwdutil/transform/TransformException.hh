#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   namespace iofwdtransform
   {
      //=====================================================================

      class TransformException : public ZException
      {
         public:
            TransformException (const std::string & msg);

      };


      //=====================================================================
   }
}
