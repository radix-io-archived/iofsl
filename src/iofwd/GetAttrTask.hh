#ifndef IOFWD_GETATTRTASK_HH
#define IOFWD_GETATTRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "GetAttrRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class GetAttrTask : public TaskHelper<GetAttrRequest>, public iofwdutil::InjectPool<GetAttrTask>
{
public:
   GetAttrTask (ThreadTaskParam & p)
      : TaskHelper<GetAttrRequest>(p)
   {
   }
   virtual ~GetAttrTask()
   {
   }

   void run ()
   {
       const GetAttrRequest::ReqParam & p = request_.decodeParam ();
       int ret = api_->getattr (p.handle, p.attr, p.op_hint);
       request_.setReturnCode (ret);
       request_.reply ((block_), (ret  == zoidfs::ZFS_OK ? p.attr : 0));
       block_.wait ();
  }

};

}

#endif
