#ifndef IOFWD_TASKSM_GETATTRTASKSM_HH
#define IOFWD_TASKSM_GETATTRTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/GetAttrRequest.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/tasksm/SharedData.hh"

namespace iofwd
{
    namespace tasksm
    {

class GetAttrTaskSM : public BaseTaskSM,
                      public iofwdutil::InjectPool< GetAttrTaskSM >
{
    public:
        GetAttrTaskSM (Request * request, const SharedData & shared)
            : BaseTaskSM(shared), ret_(0),
              request_(static_cast<GetAttrRequest *>(request))
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
            api_->getattr(slots_[BASE_SLOT], &ret_, p_.handle, p_.attr,
                    (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &GetAttrTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_->setReturnCode(ret_);
            request_->reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? p_.attr : 0));
            slots_.wait(BASE_SLOT, &GetAttrTaskSM::waitReply);
        }

    protected:
        int ret_;
        boost::scoped_ptr<GetAttrRequest> request_;
        GetAttrRequest::ReqParam p_;
};

    }
}
#endif
