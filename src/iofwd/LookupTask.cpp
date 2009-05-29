#include "LookupTask.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

using namespace zoidfs; 

namespace iofwd
{
//===========================================================================


void LookupTask::run()
{
   const LookupRequest::ReqParam & p = request_.decodeParam (); 
   zoidfs_handle_t handle; 
   int ret = api_->lookup (p.parent_handle, p.component_name, 
         p.full_path, &handle); 
   request_.setReturnCode (ret); 
   request_.reply ( (ret  == ZFS_OK ? &handle : 0)); 
}


//===========================================================================
}
