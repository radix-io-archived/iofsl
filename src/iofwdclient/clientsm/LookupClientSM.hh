#ifndef IOFWDCLIENT_SM_LookupClientSM
#define IOFWDCLIENT_SM_LookupClientSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"
#include "iofwdclient/streamwrappers/LookupStreams.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"

#include "zoidfs/zoidfs.h"

#include <cstdio>

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {

class LookupClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::LookupClientSM >
{
    public:
        LookupClientSM(sm::SMManager & smm,
                bool poll,
                const IOFWDClientCB & cb,
                net::AddressPtr addr,
                int * ret,
                const zoidfs::zoidfs_handle_t *parent_handle,
                const char *component_name, 
                const char *full_path,
                zoidfs::zoidfs_handle_t *handle,  
                zoidfs::zoidfs_op_hint_t * op_hint) :
            sm::SimpleSM< iofwdclient::clientsm::LookupClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            in_(LookupInStream(parent_handle, component_name, full_path, op_hint)),
            out_(handle, op_hint),
            server_sm_(NULL)
        {
            addr_ = addr;
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~LookupClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::LookupClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;

        streamwrappers::LookupInStream in_;
        streamwrappers::LookupOutStream out_;
        net::AddressPtr addr_;
        sm::SMClient * server_sm_;
};

    }
}

#endif
