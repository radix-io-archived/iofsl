#ifndef IOFWDUTIL_FILESPECHELPER_HH
#define IOFWDUTIL_FILESPECHELPER_HH

#include "zoidfs-wrapped.hh"
#include "zoidfs-xdr.hh"
#include "zoidfs/zoidfs-proto.h"
#include "encoder/EncoderWrappers.hh"
#include "encoder/Util.hh"
#include <boost/utility/enable_if.hpp>

namespace zoidfs
{
//===========================================================================

   /* Strucure Definition */

   typedef encoder::EncoderString<0,ZOIDFS_PATH_MAX>  ZoidFSPathName;
   typedef encoder::EncoderString<0,ZOIDFS_COMPONENT_MAX> ZoidFSComponentName;

   struct ZoidFSFileSpec
   {
       
       ZoidFSPathName full_path;
       ZoidFSComponentName component;
       zoidfs_handle_t handle;
   };


   /* Size processor */
   template <typename ENC>
   static void process (ENC e, ZoidFSFileSpec & p,
         typename only_size_processor<ENC>::type * )
   {
      uint32_t flag;
      process ( e, flag);
      /* Varibles used to calculate size */
      process ( e, p.full_path);
      process ( e, p.component);
      process ( e, p.handle);
   }

   template <typename ENC>
   static void process (ENC e, ZoidFSFileSpec & p,
                        typename only_decoder_processor<ENC>::type *)
   {
      uint32_t flag;
      process ( e, flag);

      /* Component/Handle decoding */
      if (flag == 1)
      {   
         process (e, p.component);
         /* read handle */ 
         process (e, p.handle);
      }
      /* Full path decoding */
      else
      {
         process (e, p.full_path);
      }

   }

   template <typename ENC>
   static void process (ENC e, ZoidFSFileSpec & p,
                   typename only_encoder_processor<ENC>::type *)
   {
      uint32_t flag = (p.full_path.value.empty() ? 1 : 0);
      process (e, flag);

      /* Component/Handle decoding */     
      if (flag == 1)
      {
         process (e, p.component);
         process (e, p.handle);
      }
      /* handle the full path */
      else 
      {
         process (e, p.full_path);
      }
   }
//===========================================================================
}




#endif
