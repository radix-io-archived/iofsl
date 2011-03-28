#include "iofwdclient/clientsm/LookupClientSM.hh"
//#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

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
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);    
    RPCServerSM< LookupInStream, LookupOutStream > * tmp = new RPCServerSM< LookupInStream, LookupOutStream >(smm_,
            poll_, slots_[BASE_SLOT], ZOIDFS_LOOKUP_RPC, in_, out_, addr_);
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    server_sm_ = tmp;
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    smm_.schedule(server_sm_);
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    slots_.wait(BASE_SLOT, &LookupClientSM::waitRPCServerSM);
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
}

void LookupClientSM::waitRPCServerSM(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
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
