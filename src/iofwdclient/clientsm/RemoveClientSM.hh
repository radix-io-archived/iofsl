#ifndef IOFWDCLIENT_SM_RemoveClientSM
#define IOFWDCLIENT_SM_RemoveClientSM

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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCRemoveRequest, common::RPCRemoveResponse> > RPCRemove;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class RemoveClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::RemoveClientSM >
{
    public:
        RemoveClientSM(sm::SMManager & smm,
                bool poll,
                RPCRemove comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t *handle, 
                const char *component_name, 
                const char *full_path,
                zoidfs::zoidfs_cache_hint_t * parent_hint, 
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::RemoveClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            parent_hint_(parent_hint)
        {
          if (handle != NULL)
          {
            in_.info.handle = *handle;
            in_.info.component = EncoderString(component_name);
          }
          else
          {
            in_.info.full_path = EncoderString(full_path);
          }
        }

        ~RemoveClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::RemoveClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCRemove comm_;
        common::RPCRemoveRequest in_;
        common::RPCRemoveResponse out_;
        zoidfs::zoidfs_handle_t * handle_;
        zoidfs::zoidfs_cache_hint_t * parent_hint_;
};

    }
}

#endif
