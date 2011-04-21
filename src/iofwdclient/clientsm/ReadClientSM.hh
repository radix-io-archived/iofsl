#ifndef IOFWDCLIENT_SM_READCLIENTSM
#define IOFWDCLIENT_SM_READCLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"
#include "iofwdclient/streamwrappers/WriteStreams.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommReadSM.hh"
#include "iofwdclient/streamwrappers/ReadStreams.hh"
#include "zoidfs/zoidfs.h"

#include <cstdio>

namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommReadSM<ReadInStream, ReadOutStream> > RPCCommClientSMRead;
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
                      zoidfs::zoidfs_op_hint_t * op_hint) :
            sm::SimpleSM< iofwdclient::clientsm::ReadClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            in_(ReadInStream(handle, mem_count, mem_starts, mem_sizes, 
                             file_count, file_starts, file_sizes, op_hint)),
            out_(op_hint, mem_count, mem_starts, mem_sizes)
        {
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
        streamwrappers::ReadInStream in_;
        streamwrappers::ReadOutStream out_;
};

    }
}

#endif
