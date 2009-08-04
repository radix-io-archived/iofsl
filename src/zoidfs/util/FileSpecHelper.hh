#ifndef IOFWDUTIL_FILESPECHELPER_HH
#define IOFWDUTIL_FILESPECHELPER_HH

#include "zoidfs-wrapped.hh"
#include "zoidfs-xdr.hh"
#include "zoidfs-proto.h"
#include "iofwdutil/xdr/XDRWrappers.hh"

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================
   
   // Helper functions to make encoding/decoding RPC easier

   /// Class that takes care of encoding/decoding handle, component, full 
   /// specification
   /// Note: This is only intended to be used for SERIALIZATION;
   /// It will not work with deserialization
   class FileSpecHelper
   {
   public:
      FileSpecHelper (const zoidfs::zoidfs_handle_t * handle, 
            const char * component, const char * full)
         : handle_(const_cast<zoidfs::zoidfs_handle_t*>(handle)), 
           component_(const_cast<char*>(component)), 
           full_(const_cast<char*>(full))
      {
      }

   public:
      zoidfs::zoidfs_handle_t * handle_; 
      char            * component_; 
      char            * full_; 
   }; 

   /**
    * If we have a handle, encode a 1 and store the handle and component.
    * Otherwise, store 0 and store the full path
    */
   template <typename T>
   inline void process (T & p, const FileSpecHelper & f)
   {
      zoidfs::zoidfs_null_param_t haveFullPath;
      if (static_cast<int>( T::XDRTYPE)== static_cast<int>(T::WRITER))
      {
         haveFullPath = (f.full_ != NULL) ? 1 : 0;
         process (p, haveFullPath);
         if (haveFullPath)
         {
            process (p, XDRString (f.full_, ZOIDFS_PATH_MAX)); 
         }
         else
         {
            process (p, *f.handle_); 
            process (p, XDRString (f.component_, ZOIDFS_NAME_MAX));
         }
      }
      else if (static_cast<int>( T::XDRTYPE) == 
            static_cast<int>(T::READER))
      {
         process (p, haveFullPath);
         if (haveFullPath)
         {
            BOOST_ASSERT (f.handle_); 
            process (p, XDRString (f.full_, ZOIDFS_PATH_MAX));
         }
         else
         {
            process (p, *f.handle_); 
            process (p, XDRString (f.component_, ZOIDFS_NAME_MAX)); 
         }
      }
      else if (static_cast<int>(T::XDRTYPE) == 
            static_cast<int>(T::SIZE))
      {
         // As far the size calculation is concerned: we should really have
         // know if the size calculation is for reads or writes...
         // for now, just factor in both path components
         process (p, haveFullPath);
         process (p, XDRString (f.full_, ZOIDFS_PATH_MAX));
         process (p, *f.handle_); 
         process (p, XDRString (f.component_, ZOIDFS_NAME_MAX));
      }
   }
   
//===========================================================================
   }
}




#endif
