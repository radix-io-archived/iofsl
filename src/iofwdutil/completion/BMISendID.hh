#ifndef IOFWDUTIL_COMPLETION_BMIRECEIVEID_HH
#define IOFWDUTIL_COMPLETION_BMIRECEIVEID_HH

#include "BMICompletionID.hh"

namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class BMIReceiveID : public BMICompletionID
{
public:
   bmi_size_t getReceiveSize (); 

protected:
   bmi_size_t receivesize_; 
}; 

//===========================================================================
   }
}
#endif
