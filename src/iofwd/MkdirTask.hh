#ifndef IOFWD_MKDIRTASK_HH
#define IOFWD_MKDIRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "MkdirRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class MkdirTask : public TaskHelper<MkdirRequest>, public iofwdutil::InjectPool<MkdirTask>
{
public:
   MkdirTask (ThreadTaskParam & p)
      : TaskHelper<MkdirRequest>(p)
   {
   }
   virtual ~MkdirTask()
   {
   }

   void run ()
   {
       const MkdirRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_cache_hint_t hint;
       int ret;

       api_->mkdir (block_, &ret, p.parent_handle, p.component_name,
                              p.full_path, p.sattr, &hint, p.op_hint);
       block_.wait ();

       request_.setReturnCode (ret);

       block_.reset();
       request_.reply ((block_), &hint);
       block_.wait();
  }

};

}

#endif
