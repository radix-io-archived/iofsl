#include "ZException.hh"

namespace iofwdutil
{

   class CFKeyMissingException : public ZException 
   {
      public:
         CFKeyMissingException (const std::string & keyname);


         virtual ~CFKeyMissingException ();

         const std::string & getKeyName () const
         { return keyname_; }

      protected:
         const std::string keyname_;
   };
}
