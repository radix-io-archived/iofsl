#ifndef IOFWDUTIL_BMI_BMIOP_HH
#define IOFWDUTIL_BMI_BMIOP_HH

extern "C" 
{
#include <bmi.h>
}

#include <boost/assert.hpp>
#include "BMIContext.hh"
#include "iofwdutil/tools.hh"
#include "BMIException.hh"
#include "iofwdutil/Timer.hh"

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

   // Maximum time we want to wait
   enum { WAIT_IDLE_TIME = 10 }; 

public:

   // If hasCompleted returns true it is not needed to wait/test to op any
   // longer
   bool hasCompleted () const 
   { return completed_; } 

   // Maximum time to wait (in seconds)
   size_t wait (unsigned int maxwait = WAIT_IDLE_TIME)
   {
      int ret;
      if (completed_)
         return actual_; 

      int out; 
      int outcount = sizeof (out); 
      bmi_size_t actual_size;
      void * user; 
      bmi_error_code_t error; 

      iofwdutil::Timer elapsed; 
      iofwdutil::TimeVal remaining; 
      elapsed.start (); 

      do
      {
         double remaining = double(maxwait) - (elapsed.current ().getFraction()); 
         if (remaining < 0) 
            break; 

         ret = BMI_test (op_, &outcount, &error, &actual_size, 
                  &user, static_cast<int>(remaining * iofwdutil::TimeVal::MS_PER_SECOND),
                  con_->getID());
      } while (ret == 0 && outcount == 0); 

      if (ret < 0 || error != 0)
      {
         BMI::check(ret);
         BMI::check(error);
      }
      if (!outcount)
      {
         // Timeout was triggered: throw exception
         ZTHROW (BMIException ()
               << zexception_msg("timeout in BMIOp.wait()!"));
      }

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
