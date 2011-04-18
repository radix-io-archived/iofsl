#include "iofwdclient/clientsm/WriteClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"
#include "iofwdevent/CBType.hh"
#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

WriteClientSM::~WriteClientSM()
{
}

void WriteClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&WriteClientSM::postRPCServerSM);
}

void WriteClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &WriteClientSM::waitRPCServerSM);
}

void WriteClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void WriteClientSM::postSMErrorState(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
