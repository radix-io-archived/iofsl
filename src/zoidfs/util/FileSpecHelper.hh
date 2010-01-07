#ifndef IOFWDUTIL_FILESPECHELPER_HH
#define IOFWDUTIL_FILESPECHELPER_HH

#include "zoidfs-wrapped.hh"
#include "zoidfs-xdr.hh"
#include "zoidfs/zoidfs-proto.h"
#include "encoder/EncoderWrappers.hh"
#include "encoder/Util.hh"
#include <boost/utility/enable_if.hpp>

namespace encoder
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
           full_(const_cast<char*>(full)),allow_write_(false)
      {
      }

      FileSpecHelper (zoidfs::zoidfs_handle_t * handle,
            char * component, char * full)
         : handle_(handle),
           component_(component),
           full_(full), allow_write_(true)
      {
      }


   public:
      zoidfs::zoidfs_handle_t * handle_;
      char            * component_;
      char            * full_;
      bool              allow_write_;
   };

   /**
    * If we have a handle, encode a 1 and store the handle and component.
    * Otherwise, store 0 and store the full path
    */
   template <typename T>
   inline void process (T & p, const FileSpecHelper & f,
         typename boost::enable_if<is_size_processor<T> >::type *  = 0)
   {
      zoidfs::zoidfs_null_param_t haveFullPath;
      // TODO: 
      //   implement 'clone' or reset or some other way to adjust the Size
      //   processor back to the old size. 
      //   This way, we can know which is the larger component.
      //
      // As far the size calculation is concerned: we should really
      // know if the size calculation is for reads or writes...
      // for now, just factor in both path components
      process (p, haveFullPath);
      process (p, EncString (f.full_, ZOIDFS_PATH_MAX));
      process (p, *f.handle_);
      process (p, EncString (f.component_, ZOIDFS_NAME_MAX));
   }

     /**
    * If we have a handle, encode a 1 and store the handle and component.
    * Otherwise, store 0 and store the full path
    */
   template <typename T>
   inline void process (T & p, const FileSpecHelper & f,
         typename boost::enable_if<is_encoder_processor<T> >::type *  = 0)
   {
      zoidfs::zoidfs_null_param_t haveFullPath;
      haveFullPath = (f.full_ != NULL) ? 1 : 0;
      process (p, haveFullPath);
      if (haveFullPath)
      {
         process (p, EncString (f.full_, ZOIDFS_PATH_MAX));
      }
      else
      {
         process (p, *f.handle_);
         process (p, EncString (f.component_, ZOIDFS_NAME_MAX));
      }
   }



   /**
    * If we have a handle, encode a 1 and store the handle and component.
    * Otherwise, store 0 and store the full path
    */
   template <typename T>
   inline void process (T & p, const FileSpecHelper & f,
         typename boost::enable_if<is_decoder_processor<T> >::type * = 0)
   {
      zoidfs::zoidfs_null_param_t haveFullPath;

      ALWAYS_ASSERT(f.allow_write_);

      process (p, haveFullPath);
      if (haveFullPath)
      {
         BOOST_ASSERT (f.handle_);
         process (p, EncString (f.full_, ZOIDFS_PATH_MAX));
      }
      else
      {
         process (p, *f.handle_);
         process (p, EncString (f.component_, ZOIDFS_NAME_MAX));
      }
   }

//===========================================================================
}




#endif
