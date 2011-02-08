#include "Request.hh"
#include "iofwdutil/assert.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"


namespace iofwd
{
//===========================================================================

// @TODO: Move this into the zoidsf-proto.h header
//  so that the data cannot get out of sync

/**
 * Contains valid IOFSL operation names
 */
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

/**
 * Convert operation id (opid) to operation name.
 * @param opid Operation ID number (int)
 * @return Char * Containing ASCII name for operation
 */
const char * Request::opid2Name (int opid) const
{
   if (opid < 0 ||
         opid > static_cast<int>((sizeof(opidnames)/sizeof(opidnames[0]))))
   {
      return "INVALID_OPID";
   }
   return opidnames[opid];
}

/**
 * Constructor for Request class
 * @param opid Opearation ID for operation requested 
 */
Request::Request (int opid)
   : opid_(opid), alloc_id_(0)
{
   ALWAYS_ASSERT (opid >= 0 && static_cast<size_t>(opid) <
         ((sizeof(opidnames)/sizeof(opidnames[0]))));
}

//===========================================================================
}
