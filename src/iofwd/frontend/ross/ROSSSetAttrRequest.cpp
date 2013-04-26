#include "ROSSSetAttrRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

const ROSSSetAttrRequest::ReqParam & ROSSSetAttrRequest::decodeParam()
{
   return param_;
}

void ROSSSetAttrRequest::reply(const CBType & UNUSED(cb),
        const zoidfs::zoidfs_attr_t * UNUSED(attr))
{
}

ROSSSetAttrRequest::~ROSSSetAttrRequest()
{
}

   }
}
