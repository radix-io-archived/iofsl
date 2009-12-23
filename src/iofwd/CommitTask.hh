#ifndef IOFWD_COMMITTASK_HH
#define IOFWD_COMMITTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "CommitRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class CommitTask : public TaskHelper<CommitRequest>
{
public:
   CommitTask (ThreadTaskParam & p)
      : TaskHelper<CommitRequest>(p)
   {
   }
   virtual ~CommitTask ()
   {
   }

   void run ()
   {
      const CommitRequest::ReqParam & p = request_.decodeParam ();
      int ret = api_->commit (p.handle, p.op_hint);
      request_.setReturnCode (ret);
      request_.reply (boost::ref(block_));
      block_.wait ();
   }
}; 

}


#endif
