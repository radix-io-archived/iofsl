#include "BMIException.hh"
#include "BMIError.hh"

#include <boost/format.hpp>

using namespace boost;

namespace iofwdutil
{
   namespace bmi
   {
      // ====================================================================

      std::string to_string (const bmi_error_code & c)
      {
         return str(format("BMI error: %s")
               % BMIError::errorString (c.value()));
      }

      //=====================================================================
   }
}


