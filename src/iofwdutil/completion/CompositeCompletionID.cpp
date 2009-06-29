#include "CompositeCompletionID.hh"
#include "iofwdutil/assert.hh"

using namespace std;

namespace iofwdutil
{
   namespace completion
   {

CompositeCompletionID::~CompositeCompletionID()
{
   wait();
   for(unsigned int i = 0; i < completed_ids_.size(); i++)
      delete completed_ids_[i];
}

void CompositeCompletionID::wait ()
{
   boost::mutex::scoped_lock lock(lock_);
   while (completed_ids_.size() < num_ids_) {
      while (ids_.empty())
         ready_.wait(lock);
      CompletionID * id = ids_.front();
      ids_.pop_front();
      id->wait();
      completed_ids_.push_back(id);
   }
}

bool CompositeCompletionID::test (unsigned int maxms)
{
   boost::mutex::scoped_lock lock(lock_);
   for (deque<CompletionID*>::iterator it = ids_.begin(); it != ids_.end();) {
      CompletionID * id = *it;
      if (id->test (maxms)) {
         it = ids_.erase(it);
         completed_ids_.push_back(id);
      } else {
         ++ it;
      }
   }
   return completed_ids_.size() == num_ids_;
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
