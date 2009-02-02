#ifndef IOFWDUTIL_BMI_BMIADDR_HH
#define IOFWDUTIL_BMI_BMIADDR_HH

extern "C" 
{
#include <bmi.h>
}

// Stupid: not possible to put a forward def of ostream because of the 
// (unspecified) template types...
#include <iostream>


#include "BMI.hh"


namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIAddr 
{
public:
   BMIAddr (const char * str)
   {
      BMI::check (BMI_addr_lookup (&addr_, str));
   }

   operator BMI_addr_t () const
   { return addr_; }

   /**
    * If unexpected is true, also search the unexpected list.
    * Note: this could result in returning something that cannot
    * be passed back to addr_lookup...
    */
   const char * toString (bool includeunexpected = false) const; 

private:
   void dummy (); 

protected:
   BMI_addr_t addr_; 

}; 

std::ostream & operator << (std::ostream & out, const BMIAddr & a); 

//===========================================================================
   }
}

#endif
