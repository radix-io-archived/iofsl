#ifndef IOFWDUTIL_COMPLETION_BMIRESOURCE_HH
#define IOFWDUTIL_COMPLETION_BMIRESOURCE_HH

extern "C" 
{
#include <bmi.h>
}

#include <boost/thread.hpp>

#include "Resource.hh"
#include "ContextBase.hh"
#include "IDAllocator.hh"
#include "iofwdutil/assert.hh"
#include "BMICompletionID.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class BMIResource : public Resource
{
public:
   BMIResource (); 

   void postSend (BMICompletionID * id, BMI_addr_t dest, const void * buffer, 
         bmi_size_t size, bmi_buffer_type buffer_type, 
         bmi_msg_tag_t tag, bmi_hint hints); 

   void postReceive (BMICompletionID * id, BMI_addr_t src, void * buffer, 
         bmi_size_t expected_size,  
         bmi_buffer_type buffer_type,
         bmi_msg_tag_t tag, bmi_hint hints); 

   void postSendUnexpected (BMICompletionID * id, BMI_addr_t dest, const
         void * buffer, bmi_size_t size, bmi_buffer_type buffertype,
         bmi_msg_tag_t tag, bmi_hint hints); 

   void postSendList (BMICompletionID * id, BMI_addr_t dest, const void * const *
         buffer_list, const bmi_size_t * size_list, int list_count, 
         bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
         tag, bmi_hint hints); 

   void postReceiveList (BMICompletionID * id, BMI_addr_t dest, void * const * 
         buffer_list, const bmi_size_t * size_list, int list_count, bmi_size_t
         total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t tag, bmi_hint
         hints); 

   void postSendUnexpectedList (BMICompletionID * id, 
         BMI_addr_t dest, const void * const *
         buffer_list, const bmi_size_t * size_list, int list_count, 
         bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
         tag, bmi_hint hints);

   /// Returns true if there are any requests to be completed 
   virtual bool isActive () const ; 

   /// Returns vector of completed operations ; will append
   virtual void waitAny (std::vector<CompletionID *> & completed) ; 

   /// Test for completion
   virtual void testAny (std::vector<CompletionID *> & completed, int maxms) ;

   /// Test for single item completion
   virtual bool test (CompletionID * id, int maxms) ;

   /// Wait for single item
   virtual void wait (CompletionID * id) ; 

   virtual ~BMIResource (); 
protected:
   bool testInternal (CompletionID * id, int maxms);

   bool testAnyInternal (std::vector<CompletionID *> & c, int maxms);


protected:
   BMICompletionID * castCheck (CompletionID * p)
   { 
      checkCompletionType (p); 
      return static_cast<BMICompletionID *> (p); 
   }

   inline void checkCompletionType (CompletionID * p)
   {
#ifndef NDEBUG
      dynamic_cast<BMICompletionID &> (*p); 
#endif
   }

   /// Called when a BMI post indicates the operation completed before
   /// returning
   void quickComplete (BMICompletionID * id)
   { id->completed_ = true; }

protected:

   int handleBMIError (int ret) const; 

   /// Do a quick check for BMI failure
   inline int checkBMI (int ret) const
   {
      if (ret >= 0) return ret;
      return handleBMIError (ret); 
   }

protected:

   /// BMI Context
   bmi_context_id bmictx_; 

   /// Lock for operation counter (outstanding_)
   boost::mutex lock_; 

   /// Reusable arrays for testcontext (need to have lock)
   std::vector<bmi_op_id_t> opsarray_; 
   std::vector<bmi_size_t> actualarray_; 
   std::vector<void *> userarray_; 
   std::vector<bmi_error_code_t> errorarray_; 

   unsigned int outstanding_; 
}; 

//===========================================================================
   }
}

#endif
