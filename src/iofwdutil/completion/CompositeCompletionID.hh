#ifndef IOFWDUTIL_COMPLETION_COMPOSITECOMPLETIONID_HH
#define IOFWDUTIL_COMPLETION_COMPOSITECOMPLETIONID_HH

#include <boost/thread.hpp>
#include <deque>
#include "CompletionID.hh"

namespace iofwdutil
{

   namespace completion
   {
//===========================================================================

class CompositeCompletionID : public CompletionID
{
public:

   CompositeCompletionID (unsigned int num_ids)
      : num_ids_(num_ids)
   {}
   virtual ~CompositeCompletionID();

   virtual void wait ();

   virtual bool test (unsigned int maxms);

   void addCompletionID (CompletionID * id);

protected:
   unsigned int num_ids_;

   std::deque<CompletionID*> ids_;
   std::deque<CompletionID*> completed_ids_;

   mutable boost::mutex lock_;
   boost::condition_variable ready_;
};

//===========================================================================
   }
}

#endif
