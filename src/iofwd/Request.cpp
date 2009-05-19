#include "Request.hh"
#include "zoidfs/zoidfs-wrapped.hh"


namespace iofwd
{
//===========================================================================

static const char * opidnames[] = 
{
"ZOIDFS_NULL",
"ZOIDFS_GET_ATTR",
"ZOIDFS_SET_ATTR",
"ZOIDFS_LOOKUP",
"ZOIDFS_READLINK",
"ZOIDFS_COMMIT",
"ZOIDFS_CREATE",
"ZOIDFS_REMOVE",
"ZOIDFS_RENAME",
"ZOIDFS_SYMLINK",
"ZOIDFS_MKDIR",
"ZOIDFS_READDIR",
"ZOIDFS_RESIZE",
"ZOIDFS_WRITE",
"ZOIDFS_READ",
"ZOIDFS_LINK"
}; 

const char * Request::opid2Name (int opid) const
{
   if (opid < 0 || 
         opid > static_cast<int>((sizeof(opidnames)/sizeof(opidnames[0]))))
   {
      return "INVALID_OPID"; 
   }
   return opidnames[opid]; 
}

Request::Request (int opid)
   : opid_(opid), status_(zoidfs::ZFS_OK)
{
}

Request::~Request ()
{
   // maybe log here 
}

//===========================================================================
}
