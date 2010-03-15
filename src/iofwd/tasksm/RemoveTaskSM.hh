#ifndef __IOFWD_TASKSM_REMOVETASKSM_HH__
#define __IOFWD_TASKSM_REMOVETASKSM_HH__

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/RemoveRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class RemoveTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< RemoveTaskSM >
{
    public:
        RemoveTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), request_(static_cast<RemoveRequest &>(*request))
        {
        }

        virtual ~RemoveTaskSM()
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
            api_->remove(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name,
                               p_.full_path, &hint_, p_.op_hint);
            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? &hint_ : 0));
            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitReply);
        }

    protected:
        int ret_;
        RemoveRequest & request_;
        RemoveRequest::ReqParam p_;
        zoidfs::zoidfs_cache_hint_t hint_;
};

    }
}
#endif
