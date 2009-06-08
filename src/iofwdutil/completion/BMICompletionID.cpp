#include "BMICompletionID.hh"
#include "iofwdutil/assert.hh"
#include "BMIResource.hh"

namespace iofwdutil
{
   namespace completion
   {

void BMICompletionID::wait ()
{
   ASSERT (!completed_); 
   resource_->wait (this); 
}

bool BMICompletionID::test (unsigned int maxms)
{
   ASSERT(!completed_); 
   return resource_->test (this, maxms); 
}


   }
}
