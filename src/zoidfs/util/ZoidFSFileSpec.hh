#ifndef IOFWDUTIL_ZOIDFSFILESPEC_HH
#define IOFWDUTIL_ZOIDFSFILESPEC_HH

#include "zoidfs-wrapped.hh"
#include "zoidfs-xdr.hh"
#include "zoidfs/zoidfs-proto.h"
#include "encoder/EncoderWrappers.hh"

#include "encoder/Util.hh"
#include <boost/utility/enable_if.hpp>
#include "zoidfs/zoidfs.h"
namespace zoidfs
{
//===========================================================================

   /* Strucure Definition */

   typedef encoder::EncoderString<0,ZOIDFS_PATH_MAX>  ZoidFSPathName;
   typedef encoder::EncoderString<0,ZOIDFS_PATH_MAX> ZoidFSComponentName;

   typedef struct
   {
       
       ZoidFSPathName full_path;
       ZoidFSComponentName component;
       zoidfs_handle_t handle;
   } ZoidFSFileSpec;


   /* Size processor */
   template <typename ENC>
   inline void process (ENC & e, const ZoidFSFileSpec & p,
         typename encoder::only_size_processor<ENC>::type * = 0)
   {
      uint32_t flag;
      process ( e, flag);
      /* Varibles used to calculate size */
      process ( e, p.full_path);
      process ( e, p.component);
      process ( e, p.handle);
   }

   template <typename ENC>
   inline void process (ENC & e, ZoidFSFileSpec & p,
                        typename encoder::only_decoder_processor<ENC>::type * = 0)
   {
      uint32_t flag;
      process ( e, flag);

      /* Component/Handle decoding */
      if (flag == 0)
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
   inline void process (ENC & e, const ZoidFSFileSpec & p,
                   typename encoder::only_encoder_processor<ENC>::type * = 0)
   {
      uint32_t flag = (p.full_path.value.empty() ? 1 : 0);
      process (e, flag);

      /* Component/Handle decoding */     
      if (flag == 0)
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
