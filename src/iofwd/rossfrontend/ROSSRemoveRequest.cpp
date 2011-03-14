#include "ROSSRemoveRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {
//===========================================================================

const ROSSRemoveRequest::ReqParam & ROSSRemoveRequest::decodeParam()
{
   return param_;
}

void ROSSRemoveRequest::reply(const CBType & UNUSED(cb),
        const zoidfs::zoidfs_cache_hint_t * UNUSED(parent_hint))
{
}

ROSSRemoveRequest::~ROSSRemoveRequest()
{
}


//===========================================================================
   }
}
