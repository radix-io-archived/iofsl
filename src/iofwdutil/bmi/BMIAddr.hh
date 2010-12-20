#ifndef IOFWDUTIL_BMI_BMIADDR_HH
#define IOFWDUTIL_BMI_BMIADDR_HH

extern "C" 
{
#include <bmi.h>
}

// Including iostream in a header is really bad (code bloat);
// We only need the declaration, so use iosfwd
#include <iosfwd>


namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIAddr 
{
public:
   BMIAddr (const char * str);

   BMIAddr (BMI_addr_t addr)
      : addr_ (addr)
   {
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
