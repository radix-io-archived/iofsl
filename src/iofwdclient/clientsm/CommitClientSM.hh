#ifndef IOFWDCLIENT_SM_CommitClientSM
#define IOFWDCLIENT_SM_CommitClientSM

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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCCommitRequest,common::RPCCommitResponse> > RPCCommCommit;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class CommitClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::CommitClientSM >
{
    public:
        CommitClientSM(sm::SMManager & smm,
                bool poll,
                RPCCommCommit comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t *handle,  
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::CommitClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm)
        {
          in_.handle = (zoidfs::zoidfs_handle_t) *handle;
        }

        ~CommitClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::CommitClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommCommit comm_;
        common::RPCCommitRequest in_;
        common::RPCCommitResponse out_;
};

    }
}

#endif
