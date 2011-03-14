#include "ROSSResizeRequest.hh"

namespace iofwd
{
   namespace rossfrontend
   {

ROSSResizeRequest::~ROSSResizeRequest()
{
}

const ROSSResizeRequest::ReqParam & ROSSResizeRequest::decodeParam()
{
   return param_;
}

void ROSSResizeRequest::reply(const CBType & UNUSED(cb))
{
}

   }
}
