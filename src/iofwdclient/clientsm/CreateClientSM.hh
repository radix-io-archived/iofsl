#ifndef IOFWDCLIENT_SM_CREATECLIENTSM
#define IOFWDCLIENT_SM_CREATECLIENTSM

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"
#include "iofwdclient/clientsm/RPCCommClientSM.hh"

#include "common/rpc/CommonRequest.hh"
#include "encoder/EncoderString.hh"
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-async.h"
#include "zoidfs/zoidfs-rpc.h"

#include "iofwdevent/CBType.hh"


namespace iofwdclient
{
    using namespace streamwrappers;

    namespace clientsm
    {
typedef boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::CreateRequest,common::CreateResponse> > RPCCommClientCreate;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class CreateClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::CreateClientSM >
{
    public:
        CreateClientSM(sm::SMManager & smm,
                        bool poll,
                        RPCCommClientCreate comm, 
                        const IOFWDClientCB & cb,
                        int * ret,
                        const zoidfs::zoidfs_handle_t *parent_handle,
                        const char *component_name, 
                        const char *full_path,
                       const zoidfs::zoidfs_sattr_t *sattr, 
                       zoidfs::zoidfs_handle_t *handle,
                       int *created,
                       zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)):
            sm::SimpleSM< iofwdclient::clientsm::CreateClientSM >(smm, poll),
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
          in_.attr = *sattr;
          /* Return values */
          handle_ = handle;
          created_ = created;
        }

        ~CreateClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::CreateClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCCommClientCreate comm_;
        common::CreateRequest in_;
        common::CreateResponse out_;
        zoidfs::zoidfs_handle_t * handle_;
        int * created_;
};

    }
}

#endif
