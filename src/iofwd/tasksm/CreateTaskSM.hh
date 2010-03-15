#ifndef __IOFWD_TASKSM_CREATETASKSM_HH__
#define __IOFWD_TASKSM_CREATETASKSM_HH__

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/CreateRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class CreateTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< CreateTaskSM >
{
    public:
        CreateTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), created_(0), request_(static_cast<CreateRequest &>(*request))
        {
        }

        virtual ~CreateTaskSM()
        {
            delete &request_;
        }

        virtual void postDecodeInput(int UNUSED(status))
        {
            p_ = request_.decodeParam();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(int UNUSED(status))
        {
            api_->create(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name,
                               p_.full_path, p_.attr, &handle_, &created_, p_.op_hint);
            slots_.wait(BASE_SLOT, &CreateTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? &handle_ : 0), created_);
            slots_.wait(BASE_SLOT, &CreateTaskSM::waitReply);
        }

    protected:
        int ret_;
        int created_;
        CreateRequest & request_;
        CreateRequest::ReqParam p_;
        zoidfs::zoidfs_handle_t handle_;
};

    }
}
#endif
