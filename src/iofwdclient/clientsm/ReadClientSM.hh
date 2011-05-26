#ifndef IOFWDCLIENT_SM_READCLIENTSM
#define IOFWDCLIENT_SM_READCLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommReadSM.hh"
#include "zoidfs/zoidfs.h"

#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include "iofwdevent/CBType.hh"

#include "common/rpc/CommonRequest.hh"

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommReadSM<common::RPCReadRequest, common::RPCReadResponse> > RPCCommClientSMRead;
class ReadClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::ReadClientSM >
{
    public:
        ReadClientSM(sm::SMManager & smm,
                      bool poll,
                      RPCCommClientSMRead comm, 
                      const IOFWDClientCB & cb,
                      int * ret,
                      const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                      void *mem_starts[], const size_t mem_sizes[],
                      size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                      zoidfs::zoidfs_file_ofs_t file_sizes[],
                      zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::ReadClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            mem_count_(mem_count),
            mem_starts_((char**)(mem_starts)),
            mem_sizes_((size_t*)(mem_sizes))
        {
          in_.handle = *handle;
          in_.file.file_count_ = file_count;
          in_.file.file_starts_ = (zoidfs::zoidfs_file_ofs_t *)(file_starts);
          in_.file.file_sizes_ = file_sizes;
        }

        ~ReadClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);
    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::ReadClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommClientSMRead comm_;
        common::RPCReadRequest in_;
        common::RPCReadResponse out_;
        size_t mem_count_;
        char ** mem_starts_;
        size_t * mem_sizes_;
};

    }
}

#endif
