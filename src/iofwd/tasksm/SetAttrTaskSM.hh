#ifndef IOFWD_TASKSM_SETATTRTASKSM_HH
#define IOFWD_TASKSM_SETATTRTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/SetAttrRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class SetAttrTaskSM : public BaseTaskSM,
                      public iofwdutil::InjectPool< SetAttrTaskSM >
{
    public:
        SetAttrTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : BaseTaskSM(smm, api), ret_(0),
              request_(static_cast<SetAttrRequest &>(*request))
        {
        }

        virtual ~SetAttrTaskSM()
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
            api_->setattr(slots_[BASE_SLOT], &ret_, p_.handle, p_.sattr, p_.attr, p_.op_hint);
            slots_.wait(BASE_SLOT, &SetAttrTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? p_.attr : 0));
            slots_.wait(BASE_SLOT, &SetAttrTaskSM::waitReply);
        }

    protected:
        int ret_;
        SetAttrRequest & request_;
        SetAttrRequest::ReqParam p_;
};

    }
}
#endif
