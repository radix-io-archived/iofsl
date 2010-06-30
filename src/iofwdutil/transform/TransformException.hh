#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   namespace transform
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
