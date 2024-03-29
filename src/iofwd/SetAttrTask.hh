#ifndef IOFWD_SETATTRTASK_HH
#define IOFWD_SETATTRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "SetAttrRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class SetAttrTask : public TaskHelper<SetAttrRequest>, public iofwdutil::InjectPool<SetAttrTask>
{
public:
   SetAttrTask (ThreadTaskParam & p)
      : TaskHelper<SetAttrRequest>(p)
   {
   }
   virtual ~SetAttrTask()
   {
   }

   void run ()
   {
       const SetAttrRequest::ReqParam & p = request_.decodeParam ();
       int ret;

       api_->setattr (block_, &ret, p.handle, p.sattr, p.attr, p.op_hint);
       block_.wait ();

       request_.setReturnCode (ret);

       block_.reset();
       request_.reply ((block_), (ret  == zoidfs::ZFS_OK ? p.attr : 0));
       block_.wait ();
  }

};

}

#endif
