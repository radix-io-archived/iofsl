#ifndef IOFWDCLIENT_SM_SYMLINKCLIENTSM
#define IOFWDCLIENT_SM_SYMLINKCLIENTSM

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
typedef  boost::shared_ptr< iofwdclient::clientsm::RPCCommClientSM<common::RPCSymlinkRequest, common::RPCSymlinkResponse> > RPCSymLink;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
class SymLinkClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::SymLinkClientSM >
{
    public:
        SymLinkClientSM(sm::SMManager & smm,
                bool poll,
                RPCSymLink comm, 
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs::zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                const zoidfs::zoidfs_sattr_t * sattr,
                zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                zoidfs::zoidfs_cache_hint_t * to_parent_hint,
                zoidfs::zoidfs_op_hint_t * UNUSED(op_hint)) :
            sm::SimpleSM< iofwdclient::clientsm::SymLinkClientSM >(smm, poll),
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
          in_.sattr = *sattr;
        }

        ~SymLinkClientSM();

        void init(iofwdevent::CBException e);

        void postRPCServerSM(iofwdevent::CBException e);
        void waitRPCServerSM(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::SymLinkClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        RPCSymLink comm_;
        zoidfs::zoidfs_cache_hint_t * from_parent_hint_;
        zoidfs::zoidfs_cache_hint_t * to_parent_hint_;
        common::RPCSymlinkRequest in_;
        common::RPCSymlinkResponse out_;
};

    }
}

#endif
