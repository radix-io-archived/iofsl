#include "iofwdclient/clientsm/MkdirClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

MkdirClientSM::~MkdirClientSM()
{
}

void MkdirClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&MkdirClientSM::postRPCServerSM);
}

void MkdirClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &MkdirClientSM::waitRPCServerSM);
}

void MkdirClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    e.check();
    *ret_ = out_.returnCode;
    *parent_hint_ = out_.parent_hint;
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void MkdirClientSM::postSMErrorState(iofwdevent::CBException e)
{
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
