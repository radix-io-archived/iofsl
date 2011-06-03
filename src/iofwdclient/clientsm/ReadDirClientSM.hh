#ifndef IOFWDCLIENT_SM_READDIRCLIENTSM
#define IOFWDCLIENT_SM_READDIRCLIENTSM

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
typedef  boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCReadDirRequest, common::RPCReadDirResponse> > RPCReadDir;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class ReadDirClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::ReadDirClientSM >
{
    public:
        ReadDirClientSM(sm::SMManager & smm,
                bool poll,
                RPCReadDir comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * parent_handle,
                zoidfs::zoidfs_dirent_cookie_t cookie, size_t * entry_count,
                zoidfs::zoidfs_dirent_t * entries, uint32_t flags,
                zoidfs::zoidfs_cache_hint_t * parent_hint,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::ReadDirClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            parent_hint_(parent_hint)
        {  
          in_.handle = *parent_handle;
          in_.entry_count = *entry_count;
          in_.flags = flags;
          in_.cookie = cookie;
          out_.entries.list = entries;
        }

        ~ReadDirClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::ReadDirClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCReadDir comm_;
        common::RPCReadDirRequest in_;
        common::RPCReadDirResponse out_;
        zoidfs::zoidfs_dirent_cookie_t cookie_;
        zoidfs::zoidfs_cache_hint_t * parent_hint_;
};

    }
}

#endif
