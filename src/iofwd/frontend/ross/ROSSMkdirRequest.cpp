#include "ROSSMkdirRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {

const ROSSMkdirRequest::ReqParam & ROSSMkdirRequest::decodeParam() 
{
   return param_; 
}

void ROSSMkdirRequest::reply(const CBType & UNUSED(cb),
      const zoidfs::zoidfs_cache_hint_t * UNUSED(parent_hint))
{
}

ROSSMkdirRequest::~ROSSMkdirRequest()
{
}


   }
}
