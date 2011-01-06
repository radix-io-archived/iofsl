#ifndef IOFWD_TASKSM_RESIZETASKSM_HH
#define IOFWD_TASKSM_RESIZETASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/ResizeRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class ResizeTaskSM : public BaseTaskSM,
                     public iofwdutil::InjectPool< ResizeTaskSM >
{
    public:
        ResizeTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : BaseTaskSM(smm, api), ret_(0),
              request_(static_cast<ResizeRequest &>(*request))
        {
        }

        virtual ~ResizeTaskSM()
        {
            delete &request_;
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            p_ = request_.decodeParam();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
           e.check ();
            api_->resize(slots_[BASE_SLOT], &ret_, p_.handle, p_.size, p_.op_hint);
            slots_.wait(BASE_SLOT, &ResizeTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &ResizeTaskSM::waitReply);
        }

    protected:
        int ret_;
        ResizeRequest & request_;
        ResizeRequest::ReqParam p_;
};

    }
}
#endif
