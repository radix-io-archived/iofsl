#ifndef IOFWD_GETATTRTASK_HH
#define IOFWD_GETATTRTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "GetAttrRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class GetAttrTask : public TaskHelper<GetAttrRequest>
{
public:
   GetAttrTask (ThreadTaskParam & p)
      : TaskHelper<GetAttrRequest>(p)
   {
   }
   virtual ~GetAttrTask()
   {
   }

   void run ()
   {
       const GetAttrRequest::ReqParam & p = request_.decodeParam ();
       int ret = api_->getattr (p.handle, p.attr);
       request_.setReturnCode (ret);
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ( (ret  == zoidfs::ZFS_OK ? p.attr : 0)));
       id->wait ();
  }

};

}

#endif
