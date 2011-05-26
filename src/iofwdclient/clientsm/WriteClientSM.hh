#ifndef IOFWDCLIENT_SM_WRITECLIENTSM
#define IOFWDCLIENT_SM_WRITECLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"
#include "common/rpc/CommonRequest.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommClientSM.hh"
#include "iofwdclient/clientsm/RPCCommWriteSM.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidfsFileOfsStruct.hh"
#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include "iofwdevent/CBType.hh"

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommWriteSM<common::WriteRequest, common::WriteResponse> > RPCCommClientSMWrite;
class WriteClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::WriteClientSM >
{
    public:
        WriteClientSM(sm::SMManager & smm,
                bool poll,
                RPCCommClientSMWrite comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                const void *mem_starts[], const size_t mem_sizes[],
                size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                zoidfs::zoidfs_file_ofs_t file_sizes[],
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::WriteClientSM >(smm, poll),
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

        ~WriteClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::WriteClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommClientSMWrite comm_;
        common::WriteRequest in_;
        common::WriteResponse out_;
        size_t mem_count_;
        char ** mem_starts_;
        size_t * mem_sizes_;
};

    }
}

#endif
