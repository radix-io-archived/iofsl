#ifndef IOFWDUTIL_COMPLETION_BMICOMPLETIONID_HH
#define IOFWDUTIL_COMPLETION_BMICOMPLETIONID_HH

#include "CompletionID.hh"

extern "C" 
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class BMIResource; 

class BMICompletionID : public CompletionID 
{
public:

   BMICompletionID ()
      : completed_ (false), resource_(0)
   {
   }
   
   bmi_error_code_t getError () const
   { return bmierror_; }

   bmi_size_t getActualSize () const
   { return actual_size_; } 


   /// Returns true if the operation has already completed.
   /// If so, it will no longer be returned in a testany / waitany call
   bool hasCompleted () const
   { return completed_; }

   virtual void wait (); 

   virtual bool test (unsigned int maxms); 

protected:
   friend class BMIResource; 

   bmi_op_id_t opid_; 

    bmi_error_code_t bmierror_; 

    bmi_size_t actual_size_; 

    bool completed_; 

    BMIResource * resource_; 
};

//===========================================================================
   }
}

#endif
