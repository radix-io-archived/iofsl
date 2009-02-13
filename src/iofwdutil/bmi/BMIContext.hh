#ifndef IOFWDUTIL_BMI_BMICONTEXT_HH
#define IOFWDUTIL_BMI_BMICONTEXT_HH

extern "C" 
{
#include <bmi.h>
}

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "BMIAddr.hh"
#include "BMITag.hh"

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

class BMIOp; 
class BMI; 
class BMIContext; 


class BMIContext : public boost::enable_shared_from_this<BMIContext>
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

public:
   ~BMIContext (); 

protected:
   bmi_context_id context_; 
}; 

typedef boost::shared_ptr<BMIContext> BMIContextPtr; 

//===========================================================================
   }
}

#endif
