#include "ROSSCreateRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

const ROSSCreateRequest::ReqParam & ROSSCreateRequest::decodeParam()
{
   return param_;
}

void ROSSCreateRequest::reply(const CBType & UNUSED(cb),
      const zoidfs::zoidfs_handle_t * UNUSED(handle),
      int UNUSED(created))
{
}

ROSSCreateRequest::~ROSSCreateRequest()
{
}

   }
}
