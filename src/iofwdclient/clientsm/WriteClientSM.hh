#ifndef IOFWDCLIENT_SM_WRITECLIENTSM
#define IOFWDCLIENT_SM_WRITECLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"
#include "iofwdclient/streamwrappers/WriteStreams.hh"

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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<WriteInStream,WriteOutStream> > RPCCommClientSMWrite;
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
                zoidfs::zoidfs_op_hint_t * op_hint) :
            sm::SimpleSM< iofwdclient::clientsm::WriteClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            in_(WriteInStream(handle, mem_count, mem_starts, mem_sizes, 
                               file_count, file_starts, file_sizes, op_hint)),
            out_(op_hint)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
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

        streamwrappers::WriteInStream in_;
        streamwrappers::WriteOutStream out_;
        RPCCommClientSMWrite comm_;
};

    }
}

#endif
