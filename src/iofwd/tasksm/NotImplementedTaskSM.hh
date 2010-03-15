#ifndef __IOFWD_TASKSM_NOTIMPLEMENTEDTASKSM_HH__
#define __IOFWD_TASKSM_NOTIMPLEMENTEDTASKSM_HH__

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/NotImplementedRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class NotImplementedTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< NotImplementedTaskSM >
{
    public:
        NotImplementedTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), request_(static_cast<NotImplementedRequest &>(*request))
        {
        }

        virtual ~NotImplementedTaskSM()
        {
            delete &request_;
        }

        virtual void postDecodeInput(int UNUSED(status))
        {
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(int UNUSED(status))
        {
            setNextMethod(&BaseTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &NotImplementedTaskSM::waitReply);
        }

    protected:
        NotImplementedRequest & request_;
};

    }
}
#endif
