#include "ROSSLookupRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rossfrontend
   {
const ROSSLookupRequest::ReqParam & ROSSLookupRequest::decodeParam ()
{
   return param_;
}

void ROSSLookupRequest::reply (const CBType & UNUSED(cb),
        const zoidfs::zoidfs_handle_t * UNUSED(handle))
{
}

ROSSLookupRequest::~ROSSLookupRequest ()
{
}
   }
}
