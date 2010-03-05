#ifndef IOFWD_LOOKUPTASK_HH
#define IOFWD_LOOKUPTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "LookupRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class LookupTask : public TaskHelper<LookupRequest>, public iofwdutil::InjectPool<LookupTask>
{
public:
   LookupTask (ThreadTaskParam & p)
      : TaskHelper<LookupRequest>(p)
   {
   }
   virtual ~LookupTask()
   {
   }

   void run ()
   {
       const LookupRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_handle_t handle;
       int ret = api_->lookup (p.parent_handle, p.component_name,
                               p.full_path, &handle, p.op_hint);
       request_.setReturnCode (ret);
       request_.reply ((block_), (ret  == zoidfs::ZFS_OK ? &handle : 0));
       block_.wait ();
  }

};

}

#endif
