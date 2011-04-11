#ifndef IOFWDCLIENT_SM_CREATECLIENTSM
#define IOFWDCLIENT_SM_CREATECLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"
#include "iofwdclient/streamwrappers/CreateStreams.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommClientSM.hh"
#include "zoidfs/zoidfs.h"

#include <cstdio>

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<CreateInStream,CreateOutStream> > RPCCommClientCreate;
class CreateClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::CreateClientSM >
{
    public:
        CreateClientSM(sm::SMManager & smm,
                        bool poll,
                        RPCCommClientCreate comm, 
                        const IOFWDClientCB & cb,
                        int * ret,
                        const zoidfs::zoidfs_handle_t *parent_handle,
                        const char *component_name, 
                        const char *full_path,
                       const zoidfs::zoidfs_sattr_t *sattr, 
                       zoidfs::zoidfs_handle_t *handle,
                       int *created,
                       zoidfs::zoidfs_op_hint_t * op_hint):
            sm::SimpleSM< iofwdclient::clientsm::CreateClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            in_(CreateInStream(parent_handle, component_name, full_path, sattr, op_hint)),
            out_(handle, created, op_hint)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~CreateClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::CreateClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommClientCreate comm_;
        streamwrappers::CreateInStream in_;
        streamwrappers::CreateOutStream out_;
};

    }
}

#endif
