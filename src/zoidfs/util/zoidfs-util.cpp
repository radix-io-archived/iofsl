#include <boost/format.hpp>
#include "zoidfs-util.hh"

using namespace boost; 


namespace zoidfs
{
//===========================================================================

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
      *bufptr = 0; 
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
