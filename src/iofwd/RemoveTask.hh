#ifndef IOFWD_REMOVETASK_HH
#define IOFWD_REMOVETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "RemoveRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class RemoveTask : public TaskHelper<RemoveRequest>
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
       int ret = api_->remove (p.parent_handle, p.component_name, 
                               p.full_path, &hint, p.op_hint);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ( (ret  == zoidfs::ZFS_OK ? &hint : 0)));
       id->wait ();
   }

}; 

}


#endif
