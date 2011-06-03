#include "iofwdclient/clientsm/ReadLinkClientSM.hh"

namespace iofwdclient
{
    namespace clientsm
    {

ReadLinkClientSM::~ReadLinkClientSM()
{
}

void ReadLinkClientSM::init(iofwdevent::CBException e)
{
    e.check();
    setNextMethod(&ReadLinkClientSM::postRPCServerSM);
}

void ReadLinkClientSM::postRPCServerSM(iofwdevent::CBException e)
{
    e.check();

    /* Runs the RPC Client State Machine */
    comm_->connect(in_, out_, slots_[BASE_SLOT]);

    /* Set up slot wait for completion */
    slots_.wait(BASE_SLOT, &ReadLinkClientSM::waitRPCServerSM);
}

void ReadLinkClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    e.check();
    *ret_ = out_.returnCode;
    buffer_ = (char*)out_.buffer.value.c_str();
    
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void ReadLinkClientSM::postSMErrorState(iofwdevent::CBException e)
{
    e.check();
    cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
