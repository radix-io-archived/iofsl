#ifndef IOFWDUTIL_BMI_BMICONTEXT_HH
#define IOFWDUTIL_BMI_BMICONTEXT_HH

extern "C" 
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIOp; 

class BMIContext
{
protected:
   BMIContext (); 

   // Not meant to be used
   bmi_context_id getID () const
   { 
      return context_; 
   }

   BMIOp postSend (BMIAddr dest, 
         const void * buffer, size_t size, 
         bmi_msg_tag_t tag, bmi_buffer_type type);

   BMIOp postSendUnexpected (BMIAddr dest, 
         const void * buffer, size_t size, 
         bmi_msg_tag_t tag, bmi_buffer_type type);

   BMIOp postReceive (BMIAddr source, 
         void * buffer, size_t maxsize, size_t * receivedsize); 

public:
   ~BMIContext (); 

protected:
   bmi_context_id context_; 
}; 


//===========================================================================
   }
}

#endif
