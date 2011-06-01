#ifndef IOFWDCLIENT_SM_MkdirClientSM
#define IOFWDCLIENT_SM_MkdirClientSM

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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCMkdirRequest,common::RPCMkdirResponse> > RPCCommMkdir;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class MkdirClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::MkdirClientSM >
{
    public:
        MkdirClientSM(sm::SMManager & smm,
                bool poll,
                RPCCommMkdir comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t *parent_handle,
                const char *component_name, 
                const char *full_path,
                const zoidfs::zoidfs_sattr_t * sattr,
                zoidfs::zoidfs_cache_hint_t * parent_hint,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::MkdirClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            parent_hint_(parent_hint),
            comm_(comm)
        {
          if (parent_handle != NULL)
          {
            in_.dir.handle = *parent_handle;
            in_.dir.component = EncoderString(component_name);
          }
          else
          {
            in_.dir.full_path = EncoderString(full_path);
          }
          in_.sattr = *sattr;
        }

        ~MkdirClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::MkdirClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        zoidfs::zoidfs_cache_hint_t * parent_hint_;
        RPCCommMkdir comm_;
        common::RPCMkdirRequest in_;
        common::RPCMkdirResponse out_;
        zoidfs::zoidfs_handle_t * handle_;
};

    }
}

#endif
