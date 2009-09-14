#ifndef IOFWDUTIL_ZEXCEPTION_HH
#define IOFWDUTIL_ZEXCEPTION_HH

// #include <boost/exception/exception.hpp>

#include <vector>
#include <string>

namespace iofwdutil
{

   class ZException 
   {
   public:
      ZException ();

      ZException (const std::string & s);

      void pushMsg (const std::string & msg); 

      virtual ~ZException (); 

      std::string toString () const; 

   protected:
      std::vector<std::string> msg_; 
   }; 



}


#endif
