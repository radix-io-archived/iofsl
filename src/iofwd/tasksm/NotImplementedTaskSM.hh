#ifndef IOFWD_TASKSM_NOTIMPLEMENTEDTASKSM_HH
#define IOFWD_TASKSM_NOTIMPLEMENTEDTASKSM_HH

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/NotImplementedRequest.hh"
#include "zoidfs/zoidfs.h"

#include <boost/scoped_ptr.hpp>

namespace iofwd
{
    namespace tasksm
    {

class NotImplementedTaskSM : public BaseTaskSM,
                             public iofwdutil::InjectPool< NotImplementedTaskSM >
{
    public:
        NotImplementedTaskSM ( Request * request, const SharedData & shared)
            : BaseTaskSM(shared),
              request_(static_cast<NotImplementedRequest *>(request))
        {
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_->reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &NotImplementedTaskSM::waitReply);
        }

    protected:
        boost::scoped_ptr<NotImplementedRequest> request_;
};

    }
}
#endif
