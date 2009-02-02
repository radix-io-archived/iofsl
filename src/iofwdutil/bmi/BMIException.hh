#include "ZException.hh"
#include <string>

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIException : public ZException 
{
public:
   BMIException (int error);

   std::string getBMIErrorString () const; 

protected:
   int error_; 
}; 


//===========================================================================
   }
}
