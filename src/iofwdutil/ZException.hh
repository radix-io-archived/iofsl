#ifndef IOFWDUTIL_ZEXCEPTION_HH
#define IOFWDUTIL_ZEXCEPTION_HH

// #include <boost/exception/exception.hpp>

#include <vector>
#include <string>

namespace iofwdutil
{

   /**
    * This class should be the base class for all exception classes defined in
    * this project.
    */
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
