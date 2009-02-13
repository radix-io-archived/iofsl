#include "BMIException.hh"
#include "BMIError.hh"

#include <iostream>

namespace iofwdutil
{
   namespace bmi
   {
      BMIException::BMIException (const char * msg)
      {
         pushMsg (msg); 
      }

      BMIException::BMIException (int error)
         : error_(error)
      {
         pushMsg (getBMIErrorString ()); 
      }

      std::string BMIException::getBMIErrorString () const
      {
         return BMIError::errorString (error_); 
      }




   }
}


