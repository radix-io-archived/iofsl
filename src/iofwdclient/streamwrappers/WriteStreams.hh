#ifndef IOFWDCLIENT_STREAMWRAPPERS_WRITESTREAM_HH
#define IOFWDCLIENT_STREAMWRAPPERS_WRITESTREAM_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"
#include "encoder/EncoderWrappers.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/util/FileSpecHelper.hh"
#include <cstdio>
using namespace encoder;
using namespace encoder::xdr;

namespace iofwdclient
{
    namespace streamwrappers
    {
         /*
            Stream / API arg wrappers
         */
         class WriteInStream
         {
             public:
                 WriteInStream( const zoidfs::zoidfs_handle_t *handle = NULL,
                                size_t mem_count = NULL,
                                const void *mem_starts[] = NULL,
                                const size_t mem_sizes[] = NULL,
                                size_t file_count = NULL, 
                                const zoidfs::zoidfs_file_ofs_t file_starts[] = NULL,
                                zoidfs::zoidfs_file_ofs_t file_sizes[] = NULL,
                                zoidfs::zoidfs_op_hint_t * op_hint = NULL) :
                     handle_(handle),
                     mem_count_(mem_count),
                     mem_starts_(mem_starts),
                     mem_sizes_(mem_sizes),
                     file_count_(file_count),
                     file_starts_(file_starts),
                     file_sizes_(file_sizes),
                     op_helper_(op_hint)
                 {
                 }
               const zoidfs::zoidfs_handle_t *handle_;
               size_t mem_count_;
               const void ** mem_starts_;
               const size_t * mem_sizes_;
               size_t file_count_;
               const zoidfs::zoidfs_file_ofs_t * file_starts_;
               zoidfs::zoidfs_file_ofs_t * file_sizes_;
               encoder::OpHintHelper op_helper_;
         };

      class WriteOutStream
      {
          public:
              WriteOutStream(zoidfs::zoidfs_op_hint_t * op_hint = NULL) 
              {
              }
              int returnCode;
              zoidfs::zoidfs_handle_t * handle_;
      };

/*
   encoder and decoders
*/

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, WriteInStream>::type * UNUSED(d) = NULL,
         typename only_size_processor<Enc>::type * = 0)
{
   /* Calculate the size */
   process (e, *(w.handle_));
   process (e, w.mem_count_);
   /* THIS NEEDS TO BE CHECKED */
   process (e, encoder::EncVarArray( (const char * const)(*w.mem_starts_), w.mem_count_));
   process (e, encoder::EncVarArrayHelper<const size_t, const size_t>(w.mem_sizes_, w.mem_count_));
   process (e, w.file_count_);
   process (e, encoder::EncVarArray( (const char * const)(w.file_starts_), w.file_count_));
   process (e, encoder::EncVarArrayHelper<const size_t, const size_t>((size_t *)(w.file_sizes_), w.file_count_));
   return e;
}

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, WriteInStream>::type * UNUSED(d) = NULL,
        typename only_encoder_processor<Enc>::type * = NULL)
{
   /* Encode */
   process (e, *(w.handle_));
   process (e, w.mem_count_);
   /* THIS NEEDS TO BE CHECKED */
   process (e, encoder::EncVarArray( (const char * const)(*w.mem_starts_), w.mem_count_));
   process (e, encoder::EncVarArrayHelper<const size_t, const size_t>(w.mem_sizes_, w.mem_count_));
   process (e, w.file_count_);
   process (e, encoder::EncVarArray( (const char * const)(w.file_starts_), w.file_count_));
   process (e, encoder::EncVarArrayHelper<const size_t, const size_t>((size_t *)(w.file_sizes_), w.file_count_));
   return e;
}
template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, WriteOutStream>::type * UNUSED(d) = NULL,
        typename only_decoder_processor<Enc>::type * = NULL)
{
    /* Decode */
    process(e, w.returnCode);
    return e;
}

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, WriteOutStream>::type * UNUSED(d) = NULL,
        typename only_size_processor<Enc>::type * = 0)
{
    /* Decode Size */
    process(e, w.returnCode);
    return e;
}
    }
}

#endif
