#ifndef IOFWD_READLINKTASK_HH
#define IOFWD_READLINKTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "ReadLinkRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class ReadLinkTask : public TaskHelper<ReadLinkRequest>
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
       const ReadLinkRequest::ReqParam & p = request_.decodeParam (); 
       char * buffer = new char[p.buffer_length];
       int ret = api_->readlink (p.handle, buffer, p.buffer_length);
       request_.setReturnCode (ret); 
       std::auto_ptr<iofwdutil::completion::CompletionID> id (request_.reply (buffer, p.buffer_length));
       id->wait ();
       delete[] buffer;
  }

}; 

}

#endif
