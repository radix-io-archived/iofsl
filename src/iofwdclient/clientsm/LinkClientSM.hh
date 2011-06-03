#ifndef IOFWDCLIENT_SM_LINKCLIENTSM
#define IOFWDCLIENT_SM_LINKCLIENTSM

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
typedef  boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCLinkRequest, common::RPCLinkResponse> > RPCLink;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class LinkClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::LinkClientSM >
{
    public:
        LinkClientSM(sm::SMManager & smm,
                bool poll,
                RPCLink comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs::zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::LinkClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            comm_(comm),
            from_parent_hint_(from_parent_hint),
            to_parent_hint_(to_parent_hint)
        {
          if (from_full_path != NULL)
          {
            in_.from.full_path = EncoderString(from_full_path);
          }
          else
          {
            in_.from.component = EncoderString(from_component_name);
            in_.from.handle = *from_parent_handle; 
          }

          if (to_full_path != NULL)
          {
            in_.to.full_path = EncoderString(to_full_path);
          }
          else
          {
            in_.to.component = EncoderString(to_component_name);
            in_.to.handle = *to_parent_handle;
          }
        }

        ~LinkClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::LinkClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCLink comm_;
        zoidfs::zoidfs_cache_hint_t * from_parent_hint_;
        zoidfs::zoidfs_cache_hint_t * to_parent_hint_;
        common::RPCLinkRequest in_;
        common::RPCLinkResponse out_;
};

    }
}

#endif
