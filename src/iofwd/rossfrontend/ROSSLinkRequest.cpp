#include "ROSSLinkRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

const ROSSLinkRequest::ReqParam & ROSSLinkRequest::decodeParam()
{
   return param_;
}

void ROSSLinkRequest::reply(const CBType & UNUSED(cb),
      const zoidfs::zoidfs_cache_hint_t * UNUSED(from_parent_hint),
      const zoidfs::zoidfs_cache_hint_t * UNUSED(to_parent_hint))
{
}

ROSSLinkRequest::~ROSSLinkRequest()
{
}

   }
}
