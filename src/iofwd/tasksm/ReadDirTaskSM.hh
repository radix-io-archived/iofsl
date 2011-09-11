#ifndef IOFWD_TASKSM_READDIRTASKSM_HH
#define IOFWD_TASKSM_READDIRTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/ReadDirRequest.hh"
#include "zoidfs/zoidfs.h"

#include <boost/scoped_ptr.hpp>

namespace iofwd
{
    namespace tasksm
    {

class ReadDirTaskSM : public BaseTaskSM,
                      public iofwdutil::InjectPool< ReadDirTaskSM >
{
    public:
        ReadDirTaskSM (Request * request, const SharedData & shared)
            : BaseTaskSM(shared), ret_(0),
              request_(static_cast<ReadDirRequest*>(request))
        {
            ZOIDFS_CACHE_HINT_INIT(parent_hint_);
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
            entry_count_ = p_.entry_count;
            api_->readdir(slots_[BASE_SLOT], &ret_, p_.handle, p_.cookie, &entry_count_,
                                p_.entries, p_.flags, &parent_hint_,
                                (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &ReadDirTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_->setReturnCode(ret_);
            request_->reply((slots_[BASE_SLOT]), entry_count_, p_.entries, &parent_hint_);
            slots_.wait(BASE_SLOT, &ReadDirTaskSM::waitReply);
        }

    protected:
        int ret_;
        boost::scoped_ptr<ReadDirRequest> request_;
        ReadDirRequest::ReqParam p_;
        zoidfs::zoidfs_cache_hint_t parent_hint_;
        size_t entry_count_;
};

    }
}
#endif
