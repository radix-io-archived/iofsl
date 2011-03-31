#ifndef IOFWDCLIENT_SM_GETATTRCLIENTSM
#define IOFWDCLIENT_SM_GETATTRCLIENTSM

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"

#include "zoidfs/zoidfs.h"

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include <cstdio>

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {

class GetAttrClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >
{
    public:
        GetAttrClientSM(sm::SMManager & smm,
                bool poll,
                const IOFWDClientCB & cb,
                net::AddressPtr addr,
                int * ret,
                const zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_attr_t * attr,
                zoidfs::zoidfs_op_hint_t * op_hint) :
            sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            in_(GetAttrInStream(handle, op_hint)),
            out_(attr, op_hint),
            server_sm_(NULL)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
            addr_ = addr;
        }

        ~GetAttrClientSM();
        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::GetAttrClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;

        GetAttrInStream in_;
        GetAttrOutStream out_;
        net::AddressPtr addr_;
        sm::SMClientSharedPtr server_sm_;
};

    }
}

#endif
