#include "iofwdclient/clientsm/LookupClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"
#include "iofwdevent/CBType.hh"
#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

LookupClientSM::~LookupClientSM()
{
}

void LookupClientSM::init(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&LookupClientSM::postRPCServerSM);
}

void LookupClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "LookupClientSM:%s:%i\n", __func__, __LINE__);
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &LookupClientSM::waitRPCServerSM);
}

void LookupClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "LookupClientSM:%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void LookupClientSM::postSMErrorState(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
