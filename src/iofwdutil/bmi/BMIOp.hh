#ifndef IOFWDUTIL_BMI_BMIOP_HH
#define IOFWDUTIL_BMI_BMIOP_HH

extern "C" 
{
#include <bmi.h>
}

#include <boost/assert.hpp>
#include "BMIContext.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
   namespace bmi
   {


class BMIOp
{
protected:
   friend class BMIContext; 

   BMIOp (BMIContextPtr con, bmi_op_id_t op)
      : con_(con), op_ (op), completed_(false)
   {
   }

   BMIOp (BMIContextPtr con, bmi_op_id_t UNUSED(op), size_t actual)
      : con_(con), actual_(actual), completed_(true)
   {
   }


protected:
   enum { WAIT_IDLE_TIME = 1000000 }; 
public:
   size_t wait ()
   {
      if (completed_)
         return actual_; 

      int out; 
      int outcount = sizeof (out); 
      bmi_size_t actual_size;
      void * user; 
      bmi_error_code_t error; 
      BMI::check(BMI_test (op_, &outcount, &error, &actual_size, 
            &user, WAIT_IDLE_TIME, con_->getID()));

      return actual_size; 
   }

   bool test (const BMIContext & UNUSED(con), size_t & size)
   {
      if (completed_)
      {
         size = actual_;
         return true; 
      }

      BOOST_ASSERT(false); 
   }

protected:
   BMIContextPtr con_; 
   union
   {
   bmi_op_id_t op_; 
   size_t actual_;
   }; 
   bool completed_; 
}; 


   }
}


#endif
