#ifndef IOFWD_TASKSM_GETATTRTASKSM_HH
#define IOFWD_TASKSM_GETATTRTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/GetAttrRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class GetAttrTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< GetAttrTaskSM >
{
    public:
        GetAttrTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), request_(static_cast<GetAttrRequest &>(*request))
        {
        }

        virtual ~GetAttrTaskSM()
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
            api_->getattr(slots_[BASE_SLOT], &ret_, p_.handle, p_.attr, p_.op_hint);
            slots_.wait(BASE_SLOT, &GetAttrTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? p_.attr : 0));
            slots_.wait(BASE_SLOT, &GetAttrTaskSM::waitReply);
        }

    protected:
        int ret_;
        GetAttrRequest & request_;
        GetAttrRequest::ReqParam p_;
};

    }
}
#endif
