#include "WorkQueueCompletionID.hh"
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

bool WorkQueueCompletionID::test (unsigned int maxms)
{
   ALWAYS_ASSERT(tracker_);
   return tracker_->test (tracker_id_);
}

   }
}
