#ifndef TEST_BMIINIT_HH
#define TEST_BMIINIT_HH

extern "C" {
#include <bmi.h>
}


namespace test
{


class BMIInit
{
public:
   BMIInit (const char * methods, const char * listen, int flags)
   {
      if (BMI_initialize (methods, listen, flags)<0)
      {
         throw "Error initializing BMI!";
      }
   }

   ~BMIInit ()
   {
      BMI_finalize ();
   }
};


}

#endif
