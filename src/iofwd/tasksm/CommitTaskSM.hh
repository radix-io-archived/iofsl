#ifndef IOFWD_TASKSM_COMMITTASKSM_HH
#define IOFWD_TASKSM_COMMITTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/CommitRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class CommitTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< CommitTaskSM >
{
    public:
        CommitTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), request_(static_cast<CommitRequest &>(*request))
        {
        }

        virtual ~CommitTaskSM()
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
            api_->commit (slots_[BASE_SLOT], &ret_, p_.handle, p_.op_hint);
            slots_.wait(BASE_SLOT, &CommitTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &CommitTaskSM::waitReply);
        }

    protected:
        int ret_;
        CommitRequest & request_;
        CommitRequest::ReqParam p_;
};

    }
}
#endif
