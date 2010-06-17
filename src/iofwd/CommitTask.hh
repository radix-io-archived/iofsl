#ifndef IOFWD_COMMITTASK_HH
#define IOFWD_COMMITTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "CommitRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class CommitTask : public TaskHelper<CommitRequest>, public iofwdutil::InjectPool<CommitTask>
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
      int ret;
      
      api_->commit (block_, &ret, p.handle, p.op_hint);
      block_.wait ();

      request_.setReturnCode (ret);

      block_.reset();
      request_.reply (block_);
      block_.wait ();
   }
};

}


#endif
