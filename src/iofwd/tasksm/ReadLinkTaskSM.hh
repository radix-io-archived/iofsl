#ifndef IOFWD_TASKSM_READLINKTASKSM_HH
#define IOFWD_TASKSM_READLINKTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/ReadLinkRequest.hh"
#include "zoidfs/zoidfs.h"

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

namespace iofwd
{
    namespace tasksm
    {

class ReadLinkTaskSM : public BaseTaskSM,
                       public iofwdutil::InjectPool< ReadLinkTaskSM >
{
    public:
        ReadLinkTaskSM (Request * request, const SharedData & shared)
            : BaseTaskSM(shared), ret_(0),
            request_(static_cast<ReadLinkRequest*>(request)),
            bufferlen_(0)
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
            // @TODO: validate buffer_length!

            // We will not allocate more memory.
            // Maybe return error instead?
            if (p_.buffer_length > (ZOIDFS_PATH_MAX+1))
                bufferlen_ = ZOIDFS_PATH_MAX + 1;
            else
                bufferlen_ = p_.buffer_length;

            /* @TODO: use BMI buffer pool here or reuse an existing buffer? */
            buffer_.reset (new char[bufferlen_]);

            api_->readlink(slots_[BASE_SLOT], &ret_, p_.handle, buffer_.get(),
                    bufferlen_, (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &ReadLinkTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_->setReturnCode(ret_);
            request_->reply((slots_[BASE_SLOT]), buffer_.get(), bufferlen_);
            slots_.wait(BASE_SLOT, &ReadLinkTaskSM::waitReply);
        }

    protected:
        int ret_;
        boost::scoped_ptr<ReadLinkRequest> request_;
        ReadLinkRequest::ReqParam p_;
        boost::scoped_array<char> buffer_;
        size_t bufferlen_;
};

    }
}
#endif
