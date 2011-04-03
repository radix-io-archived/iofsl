#include "iofwdclient/clientsm/ReadClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"
#include "iofwdevent/CBType.hh"
#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

ReadClientSM::~ReadClientSM()
{
}

void ReadClientSM::init(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&ReadClientSM::postRPCServerSM);
}

void ReadClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "ReadClientSM:%s:%i\n", __func__, __LINE__);
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &ReadClientSM::waitRPCServerSM);
}

void ReadClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "ReadClientSM:%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void ReadClientSM::postSMErrorState(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
