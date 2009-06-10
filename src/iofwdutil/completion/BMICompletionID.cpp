#include "BMICompletionID.hh"
#include "iofwdutil/assert.hh"
#include "BMIResource.hh"

namespace iofwdutil
{
   namespace completion
   {

void BMICompletionID::wait ()
{
   if (!completed_)
      resource_->wait (this); 
}

bool BMICompletionID::test (unsigned int maxms)
{
   return completed_ ? true : resource_->test (this, maxms);
}


   }
}
