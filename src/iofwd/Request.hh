#ifndef IOFWD_REQUEST_HH
#define IOFWD_REQUEST_HH

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMITag.hh"

namespace iofwd
{
//===========================================================================

/**
 * This class represents a request from a client. It contains the necessary
 * information to reply to the client.
 *
 *
 */
class Request 
{
public:

   Request (int opid, iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMITag tag); 

   // Return the ZoidFS operation associated with this request 
   int getOpID () const
   {
      return opid_; 
   }; 

   virtual ~Request (); 
 
protected:
   // Operation
   int opid_; 

   // Where our client is located
   iofwdutil::bmi::BMIAddr addr_;
   iofwdutil::bmi::BMITag  tag_; 

}; 

//===========================================================================
}

#endif
