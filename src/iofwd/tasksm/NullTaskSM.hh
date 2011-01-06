#ifndef IOFWD_TASKSM_NULLTASKSM_HH
#define IOFWD_TASKSM_NULLTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/NullRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class NullTaskSM : public BaseTaskSM,
                   public iofwdutil::InjectPool< NullTaskSM >
{
    public:
        NullTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : BaseTaskSM(smm, api), ret_(0),
              request_(static_cast<NullRequest &>(*request))
        {
        }

        virtual ~NullTaskSM()
        {
            delete &request_;
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
           e.check ();
            api_->null(slots_[BASE_SLOT], &ret_);
            slots_.wait(BASE_SLOT, &NullTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &NullTaskSM::waitReply);
        }

    protected:
        int ret_;
        NullRequest & request_;
};

    }
}
#endif
