#ifndef ZOIDFS_UTIL_ZOIDFSFILEOFSSTRUCT
#define ZOIDFS_UTIL_ZOIDFSFILEOFSSTRUCT

#include <stdint.h>
#include "zoidfs/zoidfs.h"
#include "encoder/Util.hh"
#include "encoder/Size.hh"
#include "encoder/EncoderException.hh"
#include "encoder/EncoderWrappers.hh"
#include <boost/utility/enable_if.hpp>
namespace zoidfs {
   struct ZoidfsFileOfsStruct
   {
      size_t MAXSIZE;
      size_t file_count_;
      zoidfs::zoidfs_file_ofs_t * file_starts_;
      zoidfs::zoidfs_file_ofs_t * file_sizes_;
      std::string value;
      ZoidfsFileOfsStruct(size_t file_count,
                   zoidfs::zoidfs_file_ofs_t * file_sizes, 
                   zoidfs::zoidfs_file_ofs_t * file_starts) :
                   file_count_(file_count),
                   file_starts_(file_starts),
                   file_sizes_(file_sizes)
      { 
        MAXSIZE = 25;
      }
      ZoidfsFileOfsStruct() { MAXSIZE = 25;}
   };

   template <typename ENC>
   static void process (ENC & e, const ZoidfsFileOfsStruct & p,
         typename encoder::only_size_processor<ENC>::type * = 0)
   {
      process (e, p.file_count_);
      process (e, encoder::EncVarArray( p.file_starts_, p.MAXSIZE));
      process (e, encoder::EncVarArrayHelper<const zoidfs::zoidfs_file_ofs_t, const size_t>(p.file_sizes_, p.MAXSIZE));
   }

   template <typename ENC>
   static void process (ENC & e,  ZoidfsFileOfsStruct & p,
                        typename encoder::only_decoder_processor<ENC>::type * = 0)
   {
      process (e, p.file_count_);      
      p.file_starts_  = new zoidfs::zoidfs_file_ofs_t[p.file_count_];
      p.file_sizes_   = new zoidfs::zoidfs_file_ofs_t[p.file_count_];
      process (e, encoder::EncVarArray( p.file_starts_, p.file_count_));
      process (e, encoder::EncVarArray( p.file_sizes_,  p.file_count_));      
   }

   template <typename ENC>
   static void process (ENC & e, const ZoidfsFileOfsStruct & p,
                        typename encoder::only_encoder_processor<ENC>::type * = 0)
   {
      process (e, p.file_count_);
      process (e, encoder::EncVarArray( p.file_starts_, p.file_count_));
      process (e, encoder::EncVarArrayHelper<const zoidfs::zoidfs_file_ofs_t, const size_t>(p.file_sizes_, p.file_count_));
   }
}
#endif
