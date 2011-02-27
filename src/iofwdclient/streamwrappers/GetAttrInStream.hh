#ifndef IOFWDCLIENT_STREAMWRAPPERS_GETATTRINSTREAM_HH
#define IOFWDCLIENT_STREAMWRAPPERS_GETATTRINSTREAM_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"

using namespace encoder;
using namespace encoder::xdr;

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

template <typename Enc, typename Wrapper>
inline Enc & process (Enc & e,
        Wrapper & w,
        typename process_filter<Wrapper, GetAttrInStream>::type * UNUSED(d) = NULL)
{
    process(e, *(w.handle_));
    process(e, w.op_helper_);

    return e;
}
    }
}

#endif
