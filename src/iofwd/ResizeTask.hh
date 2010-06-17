#ifndef IOFWD_RESIZETASK_HH
#define IOFWD_RESIZETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "ResizeRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class ResizeTask : public TaskHelper<ResizeRequest>, public iofwdutil::InjectPool<ResizeTask>
{
public:
   ResizeTask (ThreadTaskParam & p)
      : TaskHelper<ResizeRequest>(p)
   {
   }
   virtual ~ResizeTask ()
   {
   }

   void run ()
   {
      const ResizeRequest::ReqParam & p = request_.decodeParam ();
      int ret;

      api_->resize (block_, &ret, p.handle, p.size, p.op_hint);
      block_.wait ();

      request_.setReturnCode (ret);

      block_.reset();
      request_.reply ((block_));
      block_.wait ();
   }
};

}

#endif
