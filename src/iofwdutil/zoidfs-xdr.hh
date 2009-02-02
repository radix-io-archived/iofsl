#ifndef IOFWDUTIL_ZOIDFS_XDR_HH
#define IOFWDUTIL_ZOIDFS_XDR_HH

#include "xdr/XDR.hh"

extern "C" 
{
#include "zoidfs.h"
}

namespace iofwdutil
{
  namespace xdr
  {
//===========================================================================


inline
XDR & operator |= (XDR & f, zoidfs_handle_t & h)
{
   f.processFixedSizeOpaque (&h.data, sizeof(h.data)); 
   return f; 
}

inline
XDR & operator |= (XDR & f, zoidfs_time_t & t)
{
   f|=t.seconds;
   f|=t.nseconds;
   return f; 
}

// C++ treats enum's as different types
// call special enum method
inline 
XDR & operator |= (XDR & f, zoidfs_attr_type_t & type)
{
   return f.processEnum(type); 
}

inline
XDR & operator |= (XDR & f, zoidfs_attr_t & t)
{
   f|=t.type; 
   f|=t.mask; 
   f|=t.mode; 
   f|=t.nlink;
   f|=t.uid; 
   f|=t.gid;
   f|=t.size;
   f|=t.blocksize;
   f|=t.fsid;
   f|=t.fileid;
   f|=t.atime;
   f|=t.mtime;
   f|=t.ctime;
   return f; 
}

inline 
XDR & operator |= (XDR & f, zoidfs_cache_hint_t & t)
{
   f|=t.size;
   f|=t.atime;
   f|=t.mtime;
   f|=t.ctime;
   return f; 
}

inline
XDR & operator |= (XDR & f, zoidfs_sattr_t & t)
{
   f|=t.mask;
   f|=t.mode;
   f|=t.uid;
   f|=t.gid;
   f|=t.size;
   f|=t.atime;
   f|=t.mtime;
}

inline 
XDR & operator |= (XDR & f, zoidfs_dirent_t & t)
{
   f.processString(t.name, sizeof(t.name));
   f|=t.handle;
   f|=t.attr;
   f|=t.cookie;
   return f; 
}


//===========================================================================
    }
}

#endif
