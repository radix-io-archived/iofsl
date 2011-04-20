#ifndef IOFWD_TASKSM_LINKTASKSM_HH
#define IOFWD_TASKSM_LINKTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/LinkRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class LinkTaskSM : public BaseTaskSM,
                   public iofwdutil::InjectPool< LinkTaskSM >
{
    public:
        LinkTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : BaseTaskSM(smm, api), ret_(0),
              request_(static_cast<LinkRequest &>(*request))
        {
        }

        virtual ~LinkTaskSM()
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
            api_->link(slots_[BASE_SLOT], &ret_, p_.from_parent_handle, p_.from_component_name, p_.from_full_path,
                             p_.to_parent_handle, p_.to_component_name, p_.to_full_path,
                             &from_parent_hint_, &to_parent_hint_,
                             (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &LinkTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), &from_parent_hint_, &to_parent_hint_);
            slots_.wait(BASE_SLOT, &LinkTaskSM::waitReply);
        }

    protected:
        int ret_;
        LinkRequest & request_;
        LinkRequest::ReqParam p_;
        zoidfs::zoidfs_cache_hint_t from_parent_hint_;
        zoidfs::zoidfs_cache_hint_t to_parent_hint_;
};

    }
}
#endif
