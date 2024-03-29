#ifndef IOFWDCLIENT_SM_LookupClientSM
#define IOFWDCLIENT_SM_LookupClientSM

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
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCLookupRequest,common::RPCLookupResponse> > RPCCommClientSMPtr;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class LookupClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::LookupClientSM >
{
    public:
        LookupClientSM(sm::SMManager & smm,
                bool poll,
                RPCCommClientSMPtr comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t *parent_handle,
                const char *component_name, 
                const char *full_path,
                zoidfs::zoidfs_handle_t *handle,  
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::LookupClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm)
        {
          if (parent_handle != NULL)
          {
            in_.info.handle = *parent_handle;
            in_.info.component = EncoderString(component_name);
          }
          else
          {
            in_.info.full_path = EncoderString(full_path);
          }
          handle_ = handle;          
        }

        ~LookupClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::LookupClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommClientSMPtr comm_;
        common::RPCLookupRequest in_;
        common::RPCLookupResponse out_;
        zoidfs::zoidfs_handle_t * handle_;
};

    }
}

#endif
