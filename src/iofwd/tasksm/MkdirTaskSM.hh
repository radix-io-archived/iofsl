#ifndef IOFWD_TASKSM_MKDIRTASKSM_HH
#define IOFWD_TASKSM_MKDIRTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/MkdirRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class MkdirTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< MkdirTaskSM >
{
    public:
        MkdirTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), request_(static_cast<MkdirRequest &>(*request))
        {
        }

        virtual ~MkdirTaskSM()
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
            api_->mkdir(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name,
                              p_.full_path, p_.sattr, &hint_, p_.op_hint);
            slots_.wait(BASE_SLOT, &MkdirTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), (&hint_));
            slots_.wait(BASE_SLOT, &MkdirTaskSM::waitReply);
        }

    protected:
        int ret_;
        MkdirRequest & request_;
        MkdirRequest::ReqParam p_;
        zoidfs::zoidfs_cache_hint_t hint_;
};

    }
}
#endif
