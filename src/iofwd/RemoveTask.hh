#ifndef IOFWD_REMOVETASK_HH
#define IOFWD_REMOVETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "RemoveRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class RemoveTask : public TaskHelper<RemoveRequest>, public iofwdutil::InjectPool<RemoveTask>
{
public:
   RemoveTask (ThreadTaskParam & p)
      : TaskHelper<RemoveRequest>(p)
   {
   }
   virtual ~RemoveTask()
   {
   }

   void run ()
   {
       const RemoveRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_cache_hint_t hint;
       int ret;

       api_->remove (block_, &ret, p.parent_handle, p.component_name,
                               p.full_path, &hint, (*p.op_hint)());
       block_.wait ();

       request_.setReturnCode (ret);

       block_.reset();
       request_.reply ((block_), (ret  == zoidfs::ZFS_OK ? &hint : 0));
       block_.wait ();
   }

};

}


#endif
