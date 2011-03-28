#include "iofwdclient/clientsm/GetAttrClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

GetAttrClientSM::~GetAttrClientSM()
{
}

void GetAttrClientSM::init(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&GetAttrClientSM::postRPCServerSM);
}

void GetAttrClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();

    server_sm_.reset(new RPCServerSM< GetAttrInStream, GetAttrOutStream >(smm_,
            poll_, slots_[BASE_SLOT], ZOIDFS_GETATTR_RPC, in_, out_, addr_));

    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitRPCServerSM);
}

void GetAttrClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void GetAttrClientSM::postSMErrorState(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    //cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
