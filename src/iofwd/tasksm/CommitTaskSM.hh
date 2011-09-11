#ifndef IOFWD_TASKSM_COMMITTASKSM_HH
#define IOFWD_TASKSM_COMMITTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/CommitRequest.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/tasksm/SharedData.hh"

namespace iofwd
{
    namespace tasksm
    {

class CommitTaskSM : public BaseTaskSM,
                     public iofwdutil::InjectPool< CommitTaskSM >
{
    public:
        CommitTaskSM (Request * request, const SharedData & shared)
            : BaseTaskSM(shared), ret_(0),
              request_(static_cast<CommitRequest *>(request))
        {
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            p_ = request_->decodeParam();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
           e.check ();
            api_->commit (slots_[BASE_SLOT], &ret_, p_.handle, (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &CommitTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_->setReturnCode(ret_);
            request_->reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &CommitTaskSM::waitReply);
        }

    protected:
        int ret_;
        boost::scoped_ptr<CommitRequest> request_;
        CommitRequest::ReqParam p_;
};

    }
}
#endif
