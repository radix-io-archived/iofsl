#ifndef ZOIDFS_ZOIDFS_UTIL_HH
#define ZOIDFS_ZOIDFS_UTIL_HH

#include <string>
#include "zoidfs-wrapped.hh"

namespace zoidfs
{
   std::string zfserror2string (int ret);

   std::string handle2string (const zoidfs_handle_t * handle);

   std::string filespec2string (const zoidfs_handle_t* parent_handle,
         const char * component, const char * full_path); 
}
#endif
