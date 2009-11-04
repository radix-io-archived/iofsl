#ifndef IOFWD_CREATETASK_HH
#define IOFWD_CREATETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "CreateRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class CreateTask : public TaskHelper<CreateRequest>
{
public:
   CreateTask (ThreadTaskParam & p)
      : TaskHelper<CreateRequest>(p)
   {
   }
   virtual ~CreateTask()
   {
   }

   void run ()
   {
       const CreateRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_handle_t handle;
       int created;
       int ret = api_->create (p.parent_handle, p.component_name,
                               p.full_path, p.attr, &handle, &created, p.op_hint);
       request_.setReturnCode (ret);
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ( (ret  == zoidfs::ZFS_OK ? &handle : 0), created));
       id->wait ();
  }

};

}

#endif
