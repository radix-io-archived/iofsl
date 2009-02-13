#ifndef IOFWDUTIL_BMI_BMITAG_HH
#define IOFWDUTIL_BMI_BMITAG_HH

extern "C" 
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace bmi
   {

      typedef bmi_msg_tag_t BMITag; 
   }
}

#endif
