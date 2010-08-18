#include "ZException.hh"

namespace iofwdutil
{

   class CFKeyMissingException : virtual public ZException
   {
      public:
         CFKeyMissingException (const std::string & keyname);

         const std::string & getKeyName () const
         { return keyname_; }

         virtual ~CFKeyMissingException () throw () {}

      protected:
         const std::string keyname_;
   };
}
