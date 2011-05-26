#ifndef IOFWDCLIENT_SM_GETATTRCLIENTSM
#define IOFWDCLIENT_SM_GETATTRCLIENTSM
#include <boost/shared_ptr.hpp>
#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdevent/CBType.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommClientSM.hh"

#include "encoder/EncoderString.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidFSFileSpec.hh"
#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include "common/rpc/CommonRequest.hh"

namespace iofwdclient
{
    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCGetAttrRequest,common::RPCGetAttrResponse> > RPCCommGetAttr;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class GetAttrClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >
{
    public:
        GetAttrClientSM(sm::SMManager & smm,
                bool poll,
                RPCCommGetAttr comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_attr_t * attr,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)):
            sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            attr_(attr),
            comm_(comm)
        {
          in_.handle = (zoidfs::zoidfs_handle_t) *handle;
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
        zoidfs::zoidfs_attr_t * attr_;
        RPCCommGetAttr comm_;
        common::RPCGetAttrRequest in_;
        common::RPCGetAttrResponse out_;
};

    }
}

#endif
