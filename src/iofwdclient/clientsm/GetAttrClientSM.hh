#ifndef IOFWDCLIENT_SM_GETATTRCLIENTSM
#define IOFWDCLIENT_SM_GETATTRCLIENTSM

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"

#include "zoidfs/zoidfs.h"

#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

class GetAttrClientSM :
    public sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >
{
    public:
        GetAttrClientSM(sm::SMManager & smm,
                bool poll,
                const IOFWDClientCB & cb,
                int * ret,
                const zoidfs::zoidfs_handle_t * handle,
                zoidfs::zoidfs_attr_t * attr,
                zoidfs::zoidfs_op_hint_t * op_hint) :
            sm::SimpleSM< iofwdclient::clientsm::GetAttrClientSM >(smm, poll),
            slots_(*this),
            cb_(cb),
            ret_(ret),
            handle_(handle),
            attr_(attr),
            op_hint_(op_hint)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        virtual ~GetAttrClientSM();

        void init(iofwdevent::CBException e);

        /* step 1 */
        void postOpenConnection(iofwdevent::CBException e);
        void waitOpenConnection(iofwdevent::CBException e);

        /* step 2 */
        void postEncodeRequest(iofwdevent::CBException e);
        void waitEncodeRequest(iofwdevent::CBException e);

        /* step 3 */
        void postFlushRequest(iofwdevent::CBException e);
        void waitFlushRequest(iofwdevent::CBException e);

        /* step 4 */
        void postWaitResponse(iofwdevent::CBException e);
        void waitWaitResponse(iofwdevent::CBException e);

        /* setp 5 */
        void postDecodeResponse(iofwdevent::CBException e);
        void waitDecodeResponse(iofwdevent::CBException e);

        void postSMErrorState(iofwdevent::CBException e);

    protected:
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::GetAttrClientSM> slots_;

        const IOFWDClientCB & cb_;
        int * ret_;
        const zoidfs::zoidfs_handle_t * handle_;
        zoidfs::zoidfs_attr_t * attr_;
        zoidfs::zoidfs_op_hint_t * op_hint_;
};

    }
}

#endif
