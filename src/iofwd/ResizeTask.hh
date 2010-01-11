#ifndef IOFWD_RESIZETASK_HH
#define IOFWD_RESIZETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "ResizeRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class ResizeTask : public TaskHelper<ResizeRequest>
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
      int ret = api_->resize (p.handle, p.size, p.op_hint);
      request_.setReturnCode (ret);
      request_.reply ((block_));
      block_.wait ();
   }
};

}

#endif
