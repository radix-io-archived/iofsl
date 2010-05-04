#ifndef ZOIDFS_ZOIDFS_OPS_HH
#define ZOIDFS_ZOIDFS_OPS_HH

#include <boost/functional/hash.hpp>
#include <cstring>
#include <iosfwd>
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace zoidfs
{
//===========================================================================

   std::ostream & operator << (std::ostream & out,
         const zoidfs_handle_t & handle);

   /**
    * Hash function for boost::unordered_xxxx and TR1 unordered.
    */
   inline std::size_t hash_value (const zoidfs_handle_t & h)
   {
      return boost::hash_range(&h.data[0], &h.data[sizeof(h.data)]);
   }

#define TMP_EQUAL_CMP(name) (t1.name == t2.name)

/**
 * Operator < for zoidfs_handle_t 
 */
inline bool operator < (const zoidfs_handle_t & h1, const zoidfs_handle_t & h2)
{
   return memcmp (&h1.data, &h2.data, sizeof(h1.data));
}
/**
 * operator == for zoidfs types.
 */
inline bool operator == (const zoidfs_time_t & t1, const zoidfs_time_t & t2)
{
   return (t1.seconds == t2.seconds) && (t1.nseconds == t2.nseconds);
}

inline bool operator == (const zoidfs_attr_t & t1, const zoidfs_attr_t & t2)
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

inline bool operator == (const zoidfs_handle_t & t1, const zoidfs_handle_t & t2)
{
   return !memcmp (&t1.data[0], &t2.data[0], sizeof (t1.data));
}


inline bool operator == (const zoidfs_cache_hint_t & t1, const zoidfs_cache_hint_t & t2)
{
   return TMP_EQUAL_CMP(size)
      && TMP_EQUAL_CMP(atime)
      && TMP_EQUAL_CMP(ctime)
      && TMP_EQUAL_CMP(mtime);
}

inline bool operator == (const zoidfs_sattr_t & t1, const zoidfs_sattr_t & t2)
{
   return TMP_EQUAL_CMP(mask)
      && TMP_EQUAL_CMP(mode)
      && TMP_EQUAL_CMP(uid)
      && TMP_EQUAL_CMP(gid)
      && TMP_EQUAL_CMP(size)
      && TMP_EQUAL_CMP(atime)
      && TMP_EQUAL_CMP(mtime);
}

inline bool operator == (const zoidfs_dirent_t & t1, const zoidfs_dirent_t & t2)
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
