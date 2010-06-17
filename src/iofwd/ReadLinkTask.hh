#ifndef IOFWD_READLINKTASK_HH
#define IOFWD_READLINKTASK_HH

#include <boost/function.hpp>
#include <vector>

#include "Task.hh"
#include "ReadLinkRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

class ReadLinkTask : public TaskHelper<ReadLinkRequest>, public iofwdutil::InjectPool<ReadLinkTask>
{
public:
   ReadLinkTask (ThreadTaskParam & p)
      : TaskHelper<ReadLinkRequest>(p)
   {
   }
   virtual ~ReadLinkTask()
   {
   }

   void run ()
   {
      size_t bufferlen;
       const ReadLinkRequest::ReqParam & p = request_.decodeParam ();

       // @TODO: validate buffer_length!

       // We will not allocate more memory.
       // Maybe return error instead?
       if (p.buffer_length > (ZOIDFS_PATH_MAX+1))
          bufferlen = ZOIDFS_PATH_MAX + 1;
       else
          bufferlen = p.buffer_length;

       char buffer[bufferlen];
       int ret;

       api_->readlink (block_, &ret, p.handle, buffer, bufferlen, p.op_hint);
       block_.wait ();

       request_.setReturnCode (ret);

       block_.reset();
       request_.reply ((block_), buffer, bufferlen);
       block_.wait ();
  }

};

}

#endif
