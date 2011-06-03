#ifndef IOFWDCLIENT_SM_READLINKCLIENTSM
#define IOFWDCLIENT_SM_READLINKCLIENTSM
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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCReadLinkRequest, common::RPCReadLinkResponse> > RPCReadLink;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class ReadLinkClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::ReadLinkClientSM >
{
    public:
        ReadLinkClientSM(sm::SMManager & smm,
                bool poll,
                RPCReadLink comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * handle,  
                char * buffer,
                size_t buffer_length,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::ReadLinkClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            buffer_(buffer)
        {
          in_.handle = *handle; 
          in_.buffer_length = buffer_length;
        }

        ~ReadLinkClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::ReadLinkClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCReadLink comm_;
        common::RPCReadLinkRequest in_;
        common::RPCReadLinkResponse out_;
        char * buffer_;
};

    }
}

#endif
