#ifndef IOFWD_RESIZETASK_HH
#define IOFWD_RESIZETASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "ResizeRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class ResizeTask : public TaskHelper<ResizeRequest>
{
public:
   ResizeTask (ThreadTaskParam & p)
      : TaskHelper<ResizeRequest>(p)
   {
   }
   virtual ~ResizeTask ()
   {
   }

   void run ()
   {
      const ResizeRequest::ReqParam & p = request_.decodeParam ();
      int ret = api_->resize (p.handle, p.size);
      request_.setReturnCode (ret);
      std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply ());
      id->wait ();
   }
}; 

}

#endif
