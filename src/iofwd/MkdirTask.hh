#ifndef IOFWD_MKDIRTASK_HH
#define IOFWD_MKDIRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "MkdirRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class MkdirTask : public TaskHelper<MkdirRequest>
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
       int ret = api_->mkdir (p.parent_handle, p.component_name,
                              p.full_path, p.sattr, &hint, p.op_hint);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply (&hint));
       id->wait ();
  }

}; 

}

#endif
