#include "iofwdclient/clientsm/CreateClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

CreateClientSM::~CreateClientSM()
{
}

void CreateClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&CreateClientSM::postRPCServerSM);
}

void CreateClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &CreateClientSM::waitRPCServerSM);
}

void CreateClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    e.check();
    *ret_ = out_.returnCode;
    *handle_ = out_.handle;
    *created_ = out_.created;
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void CreateClientSM::postSMErrorState(iofwdevent::CBException e)
{
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
