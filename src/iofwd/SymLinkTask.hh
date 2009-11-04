#ifndef IOFWD_SYMLINKTASK_HH
#define IOFWD_SYMLINKTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "SymLinkRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class SymLinkTask : public TaskHelper<SymLinkRequest>
{
public:
   SymLinkTask (ThreadTaskParam & p)
      : TaskHelper<SymLinkRequest>(p)
   {
   }
   virtual ~SymLinkTask()
   {
   }

   void run ()
   {
       const SymLinkRequest::ReqParam & p = request_.decodeParam (); 
       zoidfs::zoidfs_cache_hint_t from_parent_hint;
       zoidfs::zoidfs_cache_hint_t to_parent_hint;
       int ret = api_->symlink (p.from_parent_handle, p.from_component_name, p.from_full_path,
                                p.to_parent_handle, p.to_component_name, p.to_full_path,
                                p.sattr, &from_parent_hint, &to_parent_hint, p.op_hint);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ( &from_parent_hint, &to_parent_hint ));
       id->wait ();
  }

}; 

}

#endif
