#ifndef IOFWD_READDIRTASK_HH
#define IOFWD_READDIRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "ReadDirRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class ReadDirTask : public TaskHelper<ReadDirRequest>
{
public:
   ReadDirTask (ThreadTaskParam & p)
      : TaskHelper<ReadDirRequest>(p)
   {
   }
   virtual ~ReadDirTask()
   {
   }

   void run ()
   {
       const ReadDirRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_cache_hint_t parent_hint;
       size_t entry_count = p.entry_count;
       int ret = api_->readdir (p.handle, p.cookie, &entry_count,
                                p.entries, p.flags, &parent_hint);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply (entry_count, p.entries, &parent_hint));
       id->wait ();
  }

}; 

}

#endif
