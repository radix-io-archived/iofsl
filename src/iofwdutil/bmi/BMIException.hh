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
   BMIException (const char * msg); 

   std::string getBMIErrorString () const; 

protected:
   int error_; 
}; 


//===========================================================================
   }
}
