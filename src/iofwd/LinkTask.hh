#ifndef IOFWD_LINKTASK_HH
#define IOFWD_LINKTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "LinkRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class LinkTask : public TaskHelper<LinkRequest>, public iofwdutil::InjectPool<LinkTask>
{
public:
   LinkTask (ThreadTaskParam & p)
      : TaskHelper<LinkRequest>(p)
   {
   }
   virtual ~LinkTask()
   {
   }

   void run ()
   {
       const LinkRequest::ReqParam & p = request_.decodeParam ();
       zoidfs::zoidfs_cache_hint_t from_parent_hint;
       zoidfs::zoidfs_cache_hint_t to_parent_hint;
       int ret;

       api_->link (block_, &ret, p.from_parent_handle, p.from_component_name,
             p.from_full_path, p.to_parent_handle, p.to_component_name,
             p.to_full_path, &from_parent_hint, &to_parent_hint, p.op_hint);
       block_.wait ();

       request_.setReturnCode (ret);

       block_.reset();
       request_.reply ((block_), &from_parent_hint, &to_parent_hint );
       block_.wait ();
  }

};

}

#endif
