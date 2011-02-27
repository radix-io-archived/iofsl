#ifndef IOFWDCLIENT_STREAMWRAPPERS_GETATTRINSTREAM_HH
#define IOFWDCLIENT_STREAMWRAPPERS_GETATTRINSTREAM_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"
#include "encoder/EncoderWrappers.hh"

namespace iofwdclient
{
    namespace streamwrappers
    {

class GetAttrInStream
{
    public:
        GetAttrInStream(const zoidfs::zoidfs_handle_t * handle = NULL,
                zoidfs::zoidfs_op_hint_t * op_hint = NULL) :
            handle_(handle),
            op_helper_(op_hint)
        {
        }

        const zoidfs::zoidfs_handle_t * handle_;
        const encoder::OpHintHelper op_helper_;
};

    }
}

namespace encoder
{

template <typename Enc>
inline Enc & process (Enc & e,
        const iofwdclient::streamwrappers::GetAttrInStream & w,
        typename encoder::only_encoder_processor<Enc>::type * UNUSED(d)= 0)
{
    encoder::xdr::process(e, *(w.handle_));
    encoder::process(e, w.op_helper_);

    return e;
}

template <typename Enc>
inline Enc & process (Enc & e,
        const iofwdclient::streamwrappers::GetAttrInStream & w,
        typename encoder::only_size_processor<Enc>::type * UNUSED(d)= 0)
{
    encoder::xdr::process(e, *(w.handle_));
    encoder::process(e, w.op_helper_);

    return e;
}

}

#endif
