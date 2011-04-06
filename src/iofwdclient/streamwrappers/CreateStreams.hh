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
                     fprintf(stderr, "CreateStreams:%s:%s\n",full_path, component_name);
                 }
                 const encoder::OpHintHelper op_helper_;
                 const zoidfs::zoidfs_handle_t *parent_handle_;
                 const char *component_name_;
                 const char *full_path_; 
                 const zoidfs::zoidfs_sattr_t * sattr_;
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
              int * created_;
              zoidfs::zoidfs_handle_t * handle_;
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
//    fprintf(stderr, "LOOKUPSTERAMS:%s:%i\n", __func__, __LINE__);
//    fprintf(stderr, "LOOKUPSTERAMS:%s:%s\n",w.full_path_, w.component_name_);
//    zoidfs::zoidfs_null_param_t haveFullPath;
//    haveFullPath = (w.full_path_ != NULL) ? 1 : 0;
//    process (e, haveFullPath);
//    if (haveFullPath)
//    {
//       process (e, EncString(w.full_path_, ZOIDFS_PATH_MAX));
//       //process(e, EncOpaque (w.full_path_, strlen(w.full_path_), ZOIDFS_PATH_MAX));
//    }
//    else
//    {
//       process(e, *(w.parent_handle_));

//       process(e, EncOpaque (w.component_name_, strlen(w.component_name_), ZOIDFS_NAME_MAX));
//    }
////    process(e, w.op_helper_);

    return e;
}
template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, CreateOutStream>::type * UNUSED(d) = NULL,
        typename only_decoder_processor<Enc>::type * = NULL)
{
    fprintf(stderr, "CreateStreams:%s:%i\n", __func__, __LINE__);
    process(e, w.returnCode);
    process(e, *(w.handle_));
    process(e, *(w.created_));
//    process(e, w.op_helper_);
    fprintf(stderr, "CreateStreams:%i\n", *(w.handle_));
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
//    process(e, w.op_helper_);
    return e;
}
    }
}

#endif
