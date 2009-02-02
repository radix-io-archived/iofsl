#include "BMIException.hh"
#include "BMIError.hh"

#include <iostream>

namespace iofwdutil
{
   namespace bmi
   {

      BMIException::BMIException (int error)
         : error_(error)
      {
         
      }

      std::string BMIException::getBMIErrorString () const
      {
         return BMIError::errorString (error_); 
      }




   }
}


