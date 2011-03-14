#include "ROSSRenameRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {
const ROSSRenameRequest::ReqParam & ROSSRenameRequest::decodeParam()
{
   return param_;
}

void ROSSRenameRequest::reply(const CBType & UNUSED(cb),
      const zoidfs::zoidfs_cache_hint_t * UNUSED(from_parent_hint),
     const zoidfs::zoidfs_cache_hint_t * UNUSED(to_parent_hint))
{
}

ROSSRenameRequest::~ROSSRenameRequest()
{
}

   }
}
