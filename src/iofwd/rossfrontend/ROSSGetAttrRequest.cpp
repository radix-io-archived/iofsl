#include "ROSSGetAttrRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

const ROSSGetAttrRequest::ReqParam & ROSSGetAttrRequest::decodeParam()
{
   return param_;
}

void ROSSGetAttrRequest::reply(const CBType & UNUSED(cb),
        const zoidfs::zoidfs_attr_t * UNUSED(attr))
{
}

ROSSGetAttrRequest::~ROSSGetAttrRequest()
{
}

//===========================================================================
   }
}
