#include "ROSSCommitRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {
ROSSCommitRequest::~ROSSCommitRequest()
{
}

const ROSSCommitRequest::ReqParam & ROSSCommitRequest::decodeParam()
{
   return param_;
}

void ROSSCommitRequest::reply(const CBType & UNUSED(cb))
{
}

   }
}
