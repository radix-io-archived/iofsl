#include "ROSSReadLinkRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

const ROSSReadLinkRequest::ReqParam & ROSSReadLinkRequest::decodeParam()
{
   return param_;
}

void ROSSReadLinkRequest::reply(const CBType & UNUSED(cb),
        const char * UNUSED(buffer),
        size_t UNUSED(buffer_length))
{
}

ROSSReadLinkRequest::~ROSSReadLinkRequest()
{
}

   }
}
