#ifndef ZOIDFS_ZOIDFS_OPS_HH
#define ZOIDFS_ZOIDFS_OPS_HH

#include <cstring>
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace zoidfs
{
//===========================================================================

#define TMP_EQUAL_CMP(name) (t1.name == t2.name)

/**
 * operator == for zoidfs types.
 */
bool operator == (const zoidfs_time_t & t1, const zoidfs_time_t & t2)
{
   return (t1.seconds == t2.seconds) && (t1.nseconds == t2.nseconds);
}

bool operator == (const zoidfs_attr_t & t1, const zoidfs_attr_t & t2)
{
   // doesn't contain a string or so, so just use memcmp
   return TMP_EQUAL_CMP(type)
      && TMP_EQUAL_CMP(mask)
      && TMP_EQUAL_CMP(mode)
      && TMP_EQUAL_CMP(nlink)
      && TMP_EQUAL_CMP(uid)
      && TMP_EQUAL_CMP(gid)
      && TMP_EQUAL_CMP(size)
      && TMP_EQUAL_CMP(blocksize)
      && TMP_EQUAL_CMP(fsid)
      && TMP_EQUAL_CMP(fileid)
      && TMP_EQUAL_CMP(atime)
      && TMP_EQUAL_CMP(mtime)
      && TMP_EQUAL_CMP(ctime);
}

bool operator == (const zoidfs_handle_t & t1, const zoidfs_handle_t & t2)
{
   return !memcmp (&t1.data[0], &t2.data[0], sizeof (t1.data));
}


bool operator == (const zoidfs_cache_hint_t & t1, const zoidfs_cache_hint_t & t2)
{
   return TMP_EQUAL_CMP(size)
      && TMP_EQUAL_CMP(atime)
      && TMP_EQUAL_CMP(ctime)
      && TMP_EQUAL_CMP(mtime);
}

bool operator == (const zoidfs_sattr_t & t1, const zoidfs_sattr_t & t2)
{
   return TMP_EQUAL_CMP(mask)
      && TMP_EQUAL_CMP(mode)
      && TMP_EQUAL_CMP(uid)
      && TMP_EQUAL_CMP(gid)
      && TMP_EQUAL_CMP(size)
      && TMP_EQUAL_CMP(atime)
      && TMP_EQUAL_CMP(mtime);
}

bool operator == (const zoidfs_dirent_t & t1, const zoidfs_dirent_t & t2)
{
   return !strncmp (t1.name, t2.name, ZOIDFS_NAME_MAX)
      && t1.handle == t2.handle
      && t1.attr == t2.attr
      && t1.cookie == t2.cookie;
}

#undef TMP_EQUAL_CMP

//===========================================================================
}

#endif
