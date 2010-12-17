#include "ZException.hh"
#include <string>

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

struct BMIException : virtual public ZException {};

typedef boost::error_info<struct tag_bmi_error, int> bmi_error_code;

std::string to_string (const bmi_error_code & c);


//===========================================================================
   }
}
