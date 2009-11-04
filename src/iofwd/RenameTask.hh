#ifndef IOFWD_RENAMETASK_HH
#define IOFWD_RENAMETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "RenameRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class RenameTask : public TaskHelper<RenameRequest>
{
public:
   RenameTask (ThreadTaskParam & p)
      : TaskHelper<RenameRequest>(p)
   {
   }
   virtual ~RenameTask()
   {
   }

   void run ()
   {
       const RenameRequest::ReqParam & p = request_.decodeParam (); 
       zoidfs::zoidfs_cache_hint_t from_parent_hint;
       zoidfs::zoidfs_cache_hint_t to_parent_hint;
       int ret = api_->rename (p.from_parent_handle, p.from_component_name, p.from_full_path,
                               p.to_parent_handle, p.to_component_name, p.to_full_path,
                               &from_parent_hint, &to_parent_hint, p.op_hint);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ( &from_parent_hint, &to_parent_hint ));
       id->wait ();
  }

}; 

}

#endif
