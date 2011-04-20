#ifndef IOFWD_TASKSM_READLINKTASKSM_HH
#define IOFWD_TASKSM_READLINKTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/ReadLinkRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class ReadLinkTaskSM : public BaseTaskSM,
                       public iofwdutil::InjectPool< ReadLinkTaskSM >
{
    public:
        ReadLinkTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : BaseTaskSM(smm, api), ret_(0),
            request_(static_cast<ReadLinkRequest &>(*request)), buffer_(NULL),
            bufferlen_(0)
        {
        }

        virtual ~ReadLinkTaskSM()
        {
            delete [] buffer_;
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
            // @TODO: validate buffer_length!

            // We will not allocate more memory.
            // Maybe return error instead?
            if (p_.buffer_length > (ZOIDFS_PATH_MAX+1))
                bufferlen_ = ZOIDFS_PATH_MAX + 1;
            else
                bufferlen_ = p_.buffer_length;

            /* @TODO: use BMI buffer pool here or reuse an existing buffer? */
            buffer_ = new char[bufferlen_];

            api_->readlink(slots_[BASE_SLOT], &ret_, p_.handle, buffer_,
                    bufferlen_, (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &ReadLinkTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]), buffer_, bufferlen_);
            slots_.wait(BASE_SLOT, &ReadLinkTaskSM::waitReply);
        }

    protected:
        int ret_;
        ReadLinkRequest & request_;
        ReadLinkRequest::ReqParam p_;
        char * buffer_;
        size_t bufferlen_;
};

    }
}
#endif
