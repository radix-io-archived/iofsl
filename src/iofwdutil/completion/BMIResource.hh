#ifndef IOFWDUTIL_COMPLETION_BMIRESOURCE_HH
#define IOFWDUTIL_COMPLETION_BMIRESOURCE_HH

#include "Resource.hh"
#include "ContextBase.hh"
#include "IDAllocator.hh"
#include <bmi.h>

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class BMIResource : public Resource
{
public:
   BMIResource (ContextBase & ctx); 

   CompletionID postSend (BMI_addr_t dest, const void * buffer, 
         bmi_size_t size, bmi_buffer_type buffer_type, 
         bmi_msg_tag_t tag, bmi_hint hints); 

   CompletionID postReceive (BMI_addr_t src, void * buffer, 
         bmi_size_t expected_size, bmi_size_t * actual_size, 
         bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag, bmi_hint hints); 

   CompletionID postSendUnexpected (BMI_addr_t dest, const void * buffer, 
         bmi_size_t size, bmi_buffer_type buffertype, bmi_msg_tag tag, 
         bmi_hints hints); 

   CompletionID postSendList (BMI_addr_t dest, const void * const *
         buffer_list, const bmi_size_t * size_list, int list_count, 
         bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
         tag, bmi_hints hints); 

   CompletionID postReceiveList (BMI_addr_t dest, const void * const * 
         buffer_list, const bmi_size_t * size_list, int list_count, bmi_size_t
         total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t tag, bmi_hint
         hints); 

   CompletionID postSendUnexpectedList (BMI_addr_t dest, const void * const *
         buffer_list, const bmi_size_t * size_list, int list_count, 
         bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
         tag, bmi_hints hints);

   virtual ~BMIResource (); 

protected:

   /// Do a quick check for BMI failure
   inline int checkBMI (int ret)
   {
      if (ret >= 0) return ret; 
      return handleBMIError (ret); 
   }

protected:

   IDAllocator<bmi_op_id_t> allocator_; 

   /// BMI Context
   bmi_context_id bmictx_; 

   /// Completion context
   ContextBase & ctx_; 
}; 

//===========================================================================
   }
}

#endif
