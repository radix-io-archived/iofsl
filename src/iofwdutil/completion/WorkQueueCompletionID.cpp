#include "WorkQueueCompletionID.hh"
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace completion
   {

void WorkQueueCompletionID::wait ()
{
   ALWAYS_ASSERT(tracker_);
   tracker_->wait (tracker_id_);
}

// @TODO: Should this just ignore maxms??
bool WorkQueueCompletionID::test (unsigned int UNUSED(maxms))
{
   ALWAYS_ASSERT(tracker_);
   return tracker_->test (tracker_id_);
}

   }
}
