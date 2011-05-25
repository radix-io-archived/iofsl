#include "iofwdclient/clientsm/LookupClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"
#include "iofwdevent/CBType.hh"
#include <cstdio>
#include "encoder/EncoderStruct.hh"
#include "zoidfs/util/ZoidFSFileSpec.hh"
namespace iofwdclient
{
    namespace clientsm
    {

LookupClientSM::~LookupClientSM()
{
}

void LookupClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&LookupClientSM::postRPCServerSM);
}

void LookupClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &LookupClientSM::waitRPCServerSM);
}

void LookupClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    e.check();
    *ret_ = out_.returnCode;
    *handle_ = (out_.handle);
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void LookupClientSM::postSMErrorState(iofwdevent::CBException e)
{
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
