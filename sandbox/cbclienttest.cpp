#include <cstdio>
#include <unistd.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-async.h"

#include "iofwdclient/CBClient.hh"
#include "iofwdevent/CBException.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdutil/tools.hh"

#include <boost/thread/barrier.hpp>

boost::barrier b(2);

void test_callback(zoidfs::zoidfs_comp_mask_t mask,
        const iofwdevent::CBException & UNUSED(e))
{
    fprintf(stderr, "%s:%i callback invoked, mask == %i!\n", __func__,
            __LINE__, mask);

    b.wait();
}

int main(int UNUSED(argc),
        char ** UNUSED(argv))
{
    iofwdclient::IOFWDClientCB cb = test_callback;
    iofwdclient::CBClient c;

    int ret = 0;
    zoidfs::zoidfs_handle_t handle;
    zoidfs::zoidfs_attr_t attr;
    zoidfs::zoidfs_op_hint_t op_hint;

    /* call the cb getattr */
    c.cbgetattr(cb, &ret, &handle, &attr, &op_hint);

    b.wait();

    fprintf(stderr, "%s:%i all done!\n", __func__, __LINE__);

    return 0;
}
