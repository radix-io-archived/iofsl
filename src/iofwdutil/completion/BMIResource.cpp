#include <limits>
#include "BMICompletionID.hh"
#include "BMIResource.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

BMIResource::BMIResource ()
{
   checkBMI (BMI_open_context (&bmictx_)); 
}

BMIResource::~BMIResource ()
{
   BMI_close_context(bmictx_); 
}

int BMIResource::handleBMIError (int ret) const
{
   ASSERT(ret >=0 && "bmi error"); 
   return ret; 
}

bool BMIResource::isActive () const
{
   return outstanding_; 
}

bool BMIResource::testAnyInternal (std::vector<CompletionID *> & completed,
      int maxms)
{
   // BMI doesn't do concurrent testcontext
   boost::mutex::scoped_lock l (lock_); 

   int outcount; 

   // resize arrays
   opsarray_.resize (outstanding_); 
   actualarray_.resize(outstanding_); 
   userarray_.resize(outstanding_); 
   errorarray_.resize(outstanding_); 

   checkBMI (BMI_testcontext (outstanding_, 
            &opsarray_[0], &outcount, &errorarray_[0], 
            &actualarray_[0], &userarray_[0], maxms, 
            bmictx_)); 
   

   if (!outcount)
      return false; 

   completed.reserve (completed.size() + outcount); 
   for (int i=0; i<outcount; ++i)
   {
      BMICompletionID * id = castCheck 
         (static_cast<CompletionID *>(userarray_[i])); 
      ASSERT (id->opid_ == opsarray_[i]); 

      id->bmierror_ = errorarray_[i]; 
      id->actual_size_ = actualarray_[i]; 
      completed.push_back (id); 


   }
   ALWAYS_ASSERT(static_cast<int>(outstanding_) >= static_cast<int>(outcount)); 

   {
      boost::mutex::scoped_lock l(lock_); 
      outstanding_ -= outcount; 
   }

   return outcount;  
}

bool BMIResource::testInternal (CompletionID * iid, int maxms)
{
   BMICompletionID * id = castCheck (iid); 
   void * user; 
   int out;
   checkBMI (BMI_test(id->opid_, &out,
            &id->bmierror_, &id->actual_size_, &user, maxms, bmictx_)); 
   if (out > 0)
      ALWAYS_ASSERT((user == (void*) id)); 
   return out; 
}

/// Returns vector of completed operations ; will append
void BMIResource::waitAny (std::vector<CompletionID * > & completed)
{
   ALWAYS_ASSERT(testAnyInternal (completed, std::numeric_limits<int>::max())); 
}

/// Test for completion
void BMIResource::testAny (std::vector<CompletionID *> & completed, int maxms)
{
   testAnyInternal (completed, maxms); 
}

/// Test for single item completion
bool BMIResource::test (CompletionID * id,  int maxms)
{
   checkCompletionType (id); 
   return testInternal (id, maxms); 
}

/// Wait for single item
void BMIResource::wait (CompletionID * id)
{
   checkCompletionType (id); 

   int outcount = 0;
   do {
     outcount = testInternal (id, std::numeric_limits<int>::max());
   } while (outcount == 0);

   {
      boost::mutex::scoped_lock l (lock_); 
      ALWAYS_ASSERT(outstanding_ > 0); 
      --outstanding_; 
   }
}


void BMIResource::postSend (BMICompletionID * id, BMI_addr_t dest, const void * buffer, 
         bmi_size_t size, bmi_buffer_type buffer_type, 
         bmi_msg_tag_t tag, bmi_hint hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_send (&id->opid_, dest, buffer, size, buffer_type, tag, 
            id, bmictx_, hints)); 
   if (ret == 1)
   {
      // operation already completed
      quickComplete (id); 
   }
   else
   {
      boost::mutex::scoped_lock l (lock_);
      ++outstanding_;
   }
}

void BMIResource::postReceive (BMICompletionID * id, BMI_addr_t src, void * buffer, 
      bmi_size_t expected_size,  
      bmi_buffer_type buffer_type,
      bmi_msg_tag_t tag, bmi_hint hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_recv (&id->opid_, src, buffer, expected_size,
            &id->actual_size_, buffer_type, tag, id, bmictx_, hints)); 

   if (ret == 1)
   {
      // Already received
      quickComplete (id); 
   }
   else
   {
      boost::mutex::scoped_lock l (lock_); 
      ++outstanding_; 

   }
}

void BMIResource::postSendUnexpected (BMICompletionID * id, BMI_addr_t dest, const
      void * buffer, bmi_size_t size, bmi_buffer_type buffertype,
      bmi_msg_tag_t tag, bmi_hint hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_sendunexpected(&id->opid_, dest, buffer, size, 
            buffertype, tag, id, bmictx_, hints)); 
   if (ret == 1)
   {
      quickComplete (id); 
   }
     else
   {
      boost::mutex::scoped_lock l (lock_); 
      ++outstanding_; 

   }
 

}

void BMIResource::postSendList (BMICompletionID * id, BMI_addr_t dest, const void * const *
      buffer_list, const bmi_size_t * size_list, int list_count, 
      bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
      tag, bmi_hint hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_send_list (&id->opid_, dest,
            buffer_list, size_list, list_count, total_size, buffer_type, tag,
            id, bmictx_, hints)); 
   if (ret == 1)
   {
      quickComplete (id); 
   }
   else
   {
      boost::mutex::scoped_lock l (lock_); 
      ++outstanding_; 

   }
 
}

void BMIResource::postReceiveList (BMICompletionID * id, BMI_addr_t dest, void * const * 
      buffer_list, const bmi_size_t * size_list, int list_count, bmi_size_t
      total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t tag, bmi_hint
      hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_recv_list (&id->opid_, dest,
            buffer_list, size_list, list_count, total_size, &id->actual_size_, 
            buffer_type, tag, id, bmictx_, hints)); 
   if (ret == 1)
   {
      quickComplete (id); 
   }
   else
   {
      boost::mutex::scoped_lock l (lock_); 
      ++outstanding_; 
   }
}

void BMIResource::postSendUnexpectedList (BMICompletionID * id, 
      BMI_addr_t dest, const void * const *
      buffer_list, const bmi_size_t * size_list, int list_count, 
      bmi_size_t total_size, bmi_buffer_type buffer_type, bmi_msg_tag_t
      tag, bmi_hint hints)
{
   id->resource_ = this; 
   const int ret = checkBMI (BMI_post_sendunexpected_list (&id->opid_, 
            dest, buffer_list, size_list, list_count, total_size, buffer_type,
            tag, id, bmictx_, hints)); 
   if (ret == 1)
   {
      quickComplete (id); 
   }
   else
   {
      boost::mutex::scoped_lock l (lock_); 
      ++outstanding_; 

   }
 
}


//===========================================================================
   }
}
