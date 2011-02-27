#ifndef IOFWDCLIENT_STREAMWRAPPERS_GETATTROUTSTREAM_HH
#define IOFWDCLIENT_STREAMWRAPPERS_GETATTROUTSTREAM_HH

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

class GetAttrOutStream
{
    public:
        GetAttrOutStream(zoidfs::zoidfs_attr_t * attr = NULL,
                zoidfs::zoidfs_op_hint_t * op_hint = NULL) :
            attr_(attr),
            op_helper_(op_hint)
        {
        }

        zoidfs::zoidfs_attr_t * attr_;
        encoder::OpHintHelper op_helper_;
};
    }
}

namespace encoder
{

template <typename Enc>
inline Enc & process (Enc & e,
        iofwdclient::streamwrappers::GetAttrOutStream & w,
        typename encoder::only_decoder_processor<Enc>::type * UNUSED(d)= 0)
{
    encoder::xdr::process(e, *(w.attr_));
    //encoder::xdr::process(e, w.op_helper_);
    return e;
}

template <typename Enc>
inline Enc & process (Enc & e,
        const iofwdclient::streamwrappers::GetAttrOutStream & w,
        typename encoder::only_size_processor<Enc>::type * UNUSED(d)= 0)
{
    encoder::xdr::process(e, *(w.attr_));
    //encoder::xdr::process(e, w.op_helper_);
    return e;
}

}

#endif
