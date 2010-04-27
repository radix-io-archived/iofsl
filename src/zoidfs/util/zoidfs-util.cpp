#include <string>
#include <boost/format.hpp>
#include "iofwdutil/assert.hh"
#include "zoidfs-util.hh"

using namespace boost;


namespace zoidfs
{
   //===========================================================================

   typedef struct 
   {
      int               code;
      const char *      desc;
   } ErrorLookup;

#define ZFSERROR(a) { a, #a }

   static ErrorLookup zfserrors[] = { 
      ZFSERROR(ZFS_OK),
      ZFSERROR(ZFSERR_PERM),
      ZFSERROR(ZFSERR_NOENT),
      ZFSERROR(ZFSERR_IO),
      ZFSERROR(ZFSERR_NXIO),
      ZFSERROR(ZFSERR_NOMEM),
      ZFSERROR(ZFSERR_ACCES),
      ZFSERROR(ZFSERR_EXIST),
      ZFSERROR(ZFSERR_NODEV),
      ZFSERROR(ZFSERR_NOTDIR),
      ZFSERROR(ZFSERR_ISDIR),
      ZFSERROR(ZFSERR_INVAL),
      ZFSERROR(ZFSERR_FBIG),
      ZFSERROR(ZFSERR_NOSPC),
      ZFSERROR(ZFSERR_ROFS),
      ZFSERROR(ZFSERR_NOTIMPL),
      ZFSERROR(ZFSERR_NAMETOOLONG),
      ZFSERROR(ZFSERR_NOTEMPTY),
      ZFSERROR(ZFSERR_DQUOT),
      ZFSERROR(ZFSERR_STALE),
      ZFSERROR(ZFSERR_WFLUSH),
      ZFSERROR(ZFSERR_OTHER)
   };

#undef ZFSERROR

   std::string zfserror2string (int ret)
   {
      for (size_t i = 0; i<sizeof(zfserrors)/sizeof(zfserrors[0]); ++i)
      {
         if (zfserrors[i].code == ret)
            return std::string(zfserrors[i].desc);
      }
      return std::string (str(format("Unknown ZFS error ('%i')") % ret));
   }

   std::string handle2string (const zoidfs_handle_t  * handle)
   {
      if (!handle)
      {
         return std::string("(null)");
      }
      char buf[sizeof(handle->data)*2+1];
      char * bufptr = buf;
      for (unsigned int i=0; i<sizeof(handle->data); ++i)
      {
         unsigned char byte = handle->data[i];
         for (unsigned int j=0; j<2; ++j)
         {
            int ret = (byte >> 4) & 0xf;
            if (ret < 10)
               *bufptr++ = static_cast<char>('0' + ret);
            else
               *bufptr++ = static_cast<char>('a' + (ret - 10));
            byte = static_cast<char> (byte << 4);
         }
      }
      *bufptr++ = 0;
      ASSERT((size_t) (bufptr - &buf[0]) <= sizeof(buf));
      return std::string(buf);
   }

   static inline const char * safestring (const char * s)
   {
      return (s ? s : "(null)");
   }

   std::string filespec2string (const zoidfs_handle_t* parent_handle,
         const char * component, const char * full_path)
   {
      return str(format("handle=%s comp=%s full=%s") % handle2string
            (parent_handle) % safestring(component) % safestring(full_path));
   }

   //===========================================================================
}
