#include "iofwdutil/ZException.hh"

namespace iofwd
{
   namespace frontend
   {
      //=====================================================================

      class HashException : public iofwdutil::ZException
      {
         public:
            HashException (const std::string & msg);

      };


      //=====================================================================
   }
}
