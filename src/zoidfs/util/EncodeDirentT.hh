#ifndef ZOIDFS_UTIL_ENCODEDIRENTT
#define ZOIDFS_UTIL_ENCODEDIRENTT

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "zoidfs/zoidfs.h"
#include "encoder/Util.hh"
#include "encoder/Size.hh"
#include "encoder/EncoderException.hh"
#include "encoder/EncoderWrappers.hh"
#include "encoder/EncoderString.hh"
#include <boost/utility/enable_if.hpp>
namespace zoidfs {
   struct EncodeDirentT
   {
      zoidfs::zoidfs_dirent_t * list;
      size_t count;
      size_t maxcount;
      EncodeDirentT () {maxcount = 100;}
   };

   template <typename ENC>
   static void process (ENC & e, const EncodeDirentT & p,
         typename encoder::only_size_processor<ENC>::type * = 0)
   {
      encoder::EncoderString<0,ZOIDFS_PATH_MAX>name;
      zoidfs_handle_t handle;
      zoidfs_attr_t attr;
      zoidfs_dirent_cookie_t cookie;
      process(e, p.maxcount);
      for (size_t x = 0; x < p.maxcount; x++)
      {
        process(e, name);        
        process(e, handle);
        process(e, attr);
        process(e, cookie);
      }
   }

   template <typename ENC>
   static void process (ENC & e,  EncodeDirentT & p,
                        typename encoder::only_decoder_processor<ENC>::type * = 0)
   {
      process(e, p.count);
      for (size_t x = 0; x < p.count; x++)
      {
        encoder::EncoderString<0,ZOIDFS_PATH_MAX>name;
        zoidfs_handle_t handle;
        zoidfs_attr_t attr;
        zoidfs_dirent_cookie_t cookie;
        process(e, name);
        strcpy(p.list[x].name , name.value.c_str());
        process(e, handle);
        p.list[x].handle = handle;
        process(e, attr);
        p.list[x].attr = attr;
        process(e, cookie);
        p.list[x].cookie = cookie;
      }
   }

   template <typename ENC>
   static void process (ENC & e, const EncodeDirentT & p,
                        typename encoder::only_encoder_processor<ENC>::type * = 0)
   {
      process(e, p.count);
      for (size_t x = 0; x < p.count; x++)
      {
        encoder::EncoderString<0,ZOIDFS_NAME_MAX>name;
        process(e, encoder::EncoderString<0,ZOIDFS_NAME_MAX>(p.list[x].name));
        process(e, p.list[x].handle);
        process(e, p.list[x].attr);
        process(e, p.list[x].cookie);
      }
   }
}
#endif
