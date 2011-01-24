#ifndef IOFWDUTIL_ZOIDFS_XDR_HH
#define IOFWDUTIL_ZOIDFS_XDR_HH

// #include "xdr/XDR.hh"


#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/hints/zoidfs-hints.h"
#include "zoidfs/util/OpHintHelper.hh"
#include "encoder/Util.hh"
#include "encoder/EncoderWrappers.hh"


namespace encoder
{
  namespace xdr
  {
//===========================================================================

      // NOTE:
      //   the following functions use the process_filter template
      //   (defined in encoder/Util.hh) to avoid having to write each
      //   process function twice (one const and one normal version).
      //
      //   It works by making the second process parameter a template,
      //   but avoids ambiguity by introducing a third template parameter
      //   which will only instantiate if the second parameter has the correct
      //   type (with or without const).
      //
  template <typename T, typename P>
  inline T & process (T & f, P & h,
        typename process_filter<P,zoidfs::zoidfs_handle_t>::type * UNUSED(d)= 0)
{
   process (f, EncOpaque(&h.data, sizeof(h.data)));
   return f;
}


template <typename T, typename P>
inline T & process (T & f, P & t,
      typename process_filter<P,zoidfs::zoidfs_time_t>::type * =0)
{
   process (f,t.seconds);
   process (f,t.nseconds);
   return f;
}

// C++ treats enum's as different types
// call special enum method
template <typename T, typename P>
   inline
T & process (T & f, P & type,
      typename process_filter<P, zoidfs::zoidfs_attr_type_t>::type * = 0)
{
   process (f, EncEnum(type));
   return f;
}

template <typename T, typename P>
inline
T & process (T & f, P & t,
      typename process_filter<P, zoidfs::zoidfs_attr_t>::type * = 0)
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

template <typename T, typename P>
inline T & process (T & f, P & t,
      typename process_filter<P, zoidfs::zoidfs_cache_hint_t>::type * =0)
{
   process(f,t.size);
   process(f,t.atime);
   process(f,t.mtime);
   process(f,t.ctime);
   return f;
}

template <typename T, typename P>
inline T & process (T & f, P t,
      typename process_filter<P, zoidfs::zoidfs_op_hint_t>::type * =0)
{
    process(f, OpHintHelper(&t));
    return f;
}

template <typename T, typename P>
inline T & process (T & f, P & t,
      typename process_filter<P, zoidfs::zoidfs_sattr_t>::type * =0)
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

template <typename T, typename P>
inline
T & process (T & f, P & t,
      typename process_filter<P,zoidfs::zoidfs_dirent_t>::type * =0)
{
   // could also use sizeof(t.name) here but ZOIDFS_NAME_MAX is more accurate
   STATIC_ASSERT(sizeof (t.name) == ZOIDFS_NAME_MAX +1 );
   process(f, EncString(t.name, ZOIDFS_NAME_MAX));
   process(f, t.handle);
   process(f, t.attr);
   process(f, t.cookie);
   return f;
}


//===========================================================================
    }
}

#endif
