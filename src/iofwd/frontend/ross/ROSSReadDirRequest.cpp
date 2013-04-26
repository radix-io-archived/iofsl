#include "ROSSReadDirRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

ROSSReadDirRequest::~ROSSReadDirRequest()
{
}
   

const ROSSReadDirRequest::ReqParam & ROSSReadDirRequest::decodeParam()
{
   return param_;
}

void ROSSReadDirRequest::reply(const CBType & UNUSED(cb),
                                 uint32_t UNUSED(entry_count),
                                 zoidfs::zoidfs_dirent_t * UNUSED(entries),
                                 zoidfs::zoidfs_cache_hint_t *
                                 UNUSED(parent_hint))
{
}

   }
}
