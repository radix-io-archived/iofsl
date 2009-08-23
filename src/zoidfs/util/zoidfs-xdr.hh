#ifndef IOFWDUTIL_ZOIDFS_XDR_HH
#define IOFWDUTIL_ZOIDFS_XDR_HH

// #include "xdr/XDR.hh"

#include "iofwdutil/assert.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwdutil
{
  namespace xdr
  {
//===========================================================================

template <typename T>
inline
T & process (T & f, zoidfs::zoidfs_handle_t & h)
{
   process (f, XDROpaque(&h.data, sizeof(h.data))); 
   return f; 
}

template <typename T>
inline T & process (T & f, zoidfs::zoidfs_time_t & t)
{
   process (f,t.seconds); 
   process (f,t.nseconds);
   return f; 
}

// C++ treats enum's as different types
// call special enum method
template <typename T>
   inline 
T & process (T & f, zoidfs::zoidfs_attr_type_t & type)
{
   process (f, XDREnum(type)); 
   return f; 
}

template <typename T>
inline
T & process (T & f, zoidfs::zoidfs_attr_t & t)
{
   process(f,t.type); 
   process(f,t.mask); 
   process(f,t.mode); 
   process(f,t.nlink);
   process(f,t.uid); 
   process(f,t.gid);
   process(f,t.size);
   process(f,t.blocksize);
   process(f,t.fsid);
   process(f,t.fileid);
   process(f,t.atime);
   process(f,t.mtime);
   process(f,t.ctime);
   return f; 
}

template <typename T>
inline 
T & process (T & f, zoidfs::zoidfs_cache_hint_t & t)
{
   process(f,t.size);
   process(f,t.atime);
   process(f,t.mtime);
   process(f,t.ctime);
   return f; 
}

template <typename T>
inline
T & process (T & f, zoidfs::zoidfs_sattr_t & t)
{
   process(f,t.mask);
   process(f,t.mode);
   process(f,t.uid);
   process(f,t.gid);
   process(f,t.size);
   process(f,t.atime);
   process(f,t.mtime);
   return f; 
}

template <typename T>
inline 
T & process (T & f, zoidfs::zoidfs_dirent_t & t)
{
   // could also use sizeof(t.name) here but ZOIDFS_NAME_MAX is more accurate
   STATIC_ASSERT(sizeof (t.name) == ZOIDFS_NAME_MAX +1 ); 
   process(f, XDRString(t.name, ZOIDFS_NAME_MAX)); 
   process(f, t.handle);
   process(f, t.attr);
   process(f, t.cookie);
   return f; 
}


//===========================================================================
    }
}

#endif
