#include "CompositeCompletionID.hh"
#include "iofwdutil/assert.hh"

using namespace std;

namespace iofwdutil
{
   namespace completion
   {

void CompositeCompletionID::wait ()
{
   boost::mutex::scoped_lock lock(lock_);
   if (num_ids_ != 0) {
      while (completed_ids_.size() < num_ids_) {
         while (ids_.empty())
            ready_.wait(lock);
         CompletionID * id = ids_.front();
         ids_.pop_front();
         id->wait();
         completed_ids_.push_back(id);
      }
   } else {
      for (deque<CompletionID*>::iterator it = ids_.begin(); it != ids_.end(); ++it) {
         CompletionID * id = *it;
         id->wait();
      }
   }
}

bool CompositeCompletionID::test (unsigned int maxms)
{
   boost::mutex::scoped_lock lock(lock_);
   if (num_ids_ != 0 && num_ids_ == completed_ids_.size())
      return true;
   for (deque<CompletionID*>::iterator it = ids_.begin(); it != ids_.end(); ++it) {
      CompletionID * id = *it;
      if (id->test (maxms)) {
         ids_.erase(it);
         completed_ids_.push_back(id);
         return true;
      }
   }
   return false;
}

void CompositeCompletionID::addCompletionID (CompletionID * id)
{
   boost::mutex::scoped_lock lock(lock_);
   ids_.push_back(id);
   if (num_ids_ != 0)
      ready_.notify_all();
}

   }
}
