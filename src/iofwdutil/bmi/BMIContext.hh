#ifndef IOFWDUTIL_BMI_BMICONTEXT_HH
#define IOFWDUTIL_BMI_BMICONTEXT_HH

extern "C" 
{
#include <bmi.h>
}

#include "BMIAddr.hh"
#include "BMITag.hh"
#include "iofwdutil/IntrusiveHelper.hh"
#include <boost/smart_ptr.hpp>

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIOp; 
class BMI; 
class BMIContext; 


class BMIContext : public IntrusiveHelper
{
protected:
   friend class BMIOp; 
   friend class BMI; 

   BMIContext (bmi_context_id ctx)
      : context_(ctx)
   {
   }

   // Not meant to be used
   bmi_context_id getID () const
   { 
      return context_; 
   }

public:
   BMIOp postSend (BMIAddr dest, 
         const void * buffer, size_t size, 
         bmi_buffer_type type, BMITag tag);

   BMIOp postSendUnexpected (BMIAddr dest, 
         const void * buffer, size_t size, 
         bmi_buffer_type type, BMITag tag);

   BMIOp postReceive (BMIAddr source, 
         void * buffer, size_t maxsize, 
         bmi_buffer_type type, BMITag tag);

   BMIOp postSendList (BMIAddr dest,
         const void ** buffer_list, const size_t * size_list, size_t list_count,
         size_t total_size, bmi_buffer_type type, BMITag tag);

public:
   ~BMIContext (); 

protected:
   bmi_context_id context_; 
}; 

typedef boost::intrusive_ptr<BMIContext> BMIContextPtr; 

INTRUSIVE_PTR_HELPER(BMIContext)

//===========================================================================
   }
}

#endif
