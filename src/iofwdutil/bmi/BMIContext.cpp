#include "BMI.hh"
#include "BMIContext.hh"
#include "BMIAddr.hh"
#include "BMIOp.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================


      BMIContext::~BMIContext ()
      {
         BMI_close_context (getID()); 
      }

//===========================================================================

BMIOp BMIContext::postReceive (BMIAddr source,
      void * buffer, size_t maxsize, bmi_buffer_type buftype,
      bmi_msg_tag_t tag)
{
   bmi_op_id_t op; 
   bmi_size_t actual_size; 

   BMI::check (BMI_post_recv (&op, 
            source, buffer, maxsize, &actual_size, 
            buftype, tag, 0 /* userptr */, 
            getID(), 0 /* bmi hints */)); 
   if (actual_size)
   {
      /* already completed */ 
      return BMIOp (BMIContextPtr(this), op, actual_size); 
   }
   else
   {
      return BMIOp (BMIContextPtr(this), op); 
   }
}

BMIOp BMIContext::postSend (BMIAddr dest, 
         const void * buffer, size_t size, 
         bmi_buffer_type type, BMITag tag)
{
   bmi_op_id_t op; 
   int ret = BMI::check (BMI_post_send (&op, dest, buffer, size, type, 
            tag, 0, getID(), 0)); 
   if (ret == 0)
      return BMIOp(BMIContextPtr(this), op); 
   else
      return BMIOp(BMIContextPtr(this), op, size); 
}

BMIOp BMIContext::postSendUnexpected (BMIAddr dest,
      const void * buffer, size_t size, bmi_buffer_type type, 
      BMITag tag)
{
   bmi_op_id_t op; 
   BMI::check (BMI_post_sendunexpected(&op, 
            dest, buffer, size, type, 
            tag, 0, getID(), 0)); 
   return BMIOp (BMIContextPtr(this), op); 
}
//===========================================================================
   }
}
