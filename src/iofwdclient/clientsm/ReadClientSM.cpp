#include "iofwdclient/clientsm/ReadClientSM.hh"
namespace iofwdclient
{
    namespace clientsm
    {

ReadClientSM::~ReadClientSM()
{
}

void ReadClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&ReadClientSM::postRPCServerSM);         
}

void ReadClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_,mem_count_, mem_starts_, mem_sizes_,
                   slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &ReadClientSM::waitRPCServerSM);
}


void ReadClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    e.check();
    *ret_ = out_.returnCode;
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void ReadClientSM::postSMErrorState(iofwdevent::CBException e)
{
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
