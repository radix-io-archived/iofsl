#ifndef IOFWDCLIENT_STREAMWRAPPERS_CREATESTREAMS_HH
#define IOFWDCLIENT_STREAMWRAPPERS_CREATESTREAMS_HH

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
         class CreateInStream
         {
             public:
                 CreateInStream( const zoidfs::zoidfs_handle_t *parent_handle = NULL,
                         const char *component_name = NULL,
                         const char *full_path = NULL,
                         const zoidfs::zoidfs_sattr_t *sattr = NULL,
                         zoidfs::zoidfs_op_hint_t * op_hint = NULL) :
                     parent_handle_(parent_handle),
                     component_name_(component_name),
                     full_path_(full_path),
                     sattr_(sattr),
                     op_helper_(op_hint)
                 {
                 }
                 const zoidfs::zoidfs_handle_t *parent_handle_;
                 const char *component_name_;
                 const char *full_path_; 
                 const zoidfs::zoidfs_sattr_t * sattr_;
                 const encoder::OpHintHelper op_helper_;
         };

      class CreateOutStream
      {
          public:
              CreateOutStream(zoidfs::zoidfs_handle_t *handle = NULL,
                              int * created = NULL,
                      zoidfs::zoidfs_op_hint_t * op_hint = NULL) :
                  handle_(handle),
                  created_(created),
                  op_helper_(op_hint)
              {
              }
              int returnCode;
              zoidfs::zoidfs_handle_t * handle_;
              int * created_;
              encoder::OpHintHelper op_helper_;
      };

/*
   encoder and decoders
*/

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, CreateInStream>::type * UNUSED(d) = NULL,
         typename only_size_processor<Enc>::type * = 0)
{
   FileSpecHelper x(w.parent_handle_, w.component_name_, w.full_path_);
   process (e,x);
   process (e, *w.sattr_);
   return e;
//   zoidfs::zoidfs_null_param_t haveFullPath;
//   haveFullPath = 0;
//   process (e, haveFullPath);
//   process(e,*(w.parent_handle_));
//   process(e, EncOpaque(NULL, ZOIDFS_NAME_MAX, ZOIDFS_NAME_MAX));
//   process (e, EncString(w.full_path_, ZOIDFS_PATH_MAX));
//   process(e,w.op_helper_);
}

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, CreateInStream>::type * UNUSED(d) = NULL,
        typename only_encoder_processor<Enc>::type * = NULL)
{
   FileSpecHelper x(w.parent_handle_, w.component_name_, w.full_path_);
      process (e,x);
   process (e, *w.sattr_);

    return e;
}
template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, CreateOutStream>::type * UNUSED(d) = NULL,
        typename only_decoder_processor<Enc>::type * = NULL)
{
    process(e, w.returnCode);
    process(e, *(w.handle_));
    process(e, *(w.created_));
    return e;
}

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, CreateOutStream>::type * UNUSED(d) = NULL,
        typename only_size_processor<Enc>::type * = 0)
{
    process(e, w.returnCode);
    process(e, *(w.handle_));
    process(e, *(w.created_));
    return e;
}
    }
}

#endif
