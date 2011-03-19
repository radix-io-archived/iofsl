#ifndef IOFWD_TASKSM_CREATETASKSM_HH
#define IOFWD_TASKSM_CREATETASKSM_HH

#include "iofwd/CreateRequest.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/IOFSLKeyValueStorage.hh"
#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "zoidfs/util/ZoidFSAsync.hh"

#include "iofwdutil/InjectPool.hh"
#include "iofwd/CreateRequest.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"

namespace iofwd
{
    namespace tasksm
    {

class CreateTaskSM : public sm::SimpleSM< CreateTaskSM >,
                     public iofwdutil::InjectPool< CreateTaskSM >
{
    public:
        CreateTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request) :
                sm::SimpleSM<CreateTaskSM>(smm),
                ret_(0),
                created_(0),
                request_(static_cast<CreateRequest &>(*request)),
                api_(api),
                slots_(*this)
        {
        }

        virtual ~CreateTaskSM()
        {
            delete &request_;
        }

        void init(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&CreateTaskSM::postDecodeInput);
        }

        void waitDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&CreateTaskSM::postRunOp);
        }

        void waitRunOp(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&CreateTaskSM::postReply);
        }

        void waitReply(iofwdevent::CBException e)
        {
            e.check ();
            // done
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            p_ = request_.decodeParam();
            setNextMethod(&CreateTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
            e.check ();
            api_->create(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name,
                               p_.full_path, p_.attr, &handle_, &created_, p_.op_hint);
            slots_.wait(BASE_SLOT, &CreateTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
            e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]),
                  (ret_  == zoidfs::ZFS_OK ?  &handle_ : 0), created_);
            slots_.wait(BASE_SLOT, &CreateTaskSM::waitReply);
        }

    protected:
        int ret_;
        int created_;
        CreateRequest & request_;
        CreateRequest::ReqParam p_;
        zoidfs::zoidfs_handle_t handle_;

        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        zoidfs::util::ZoidFSAsync * api_;
        sm::SimpleSlots<NUM_BASE_SLOTS, iofwd::tasksm::CreateTaskSM> slots_;
};

    }
}
#endif
