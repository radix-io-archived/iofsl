#ifndef IOFWD_TASKSM_BASETASKSM_HH
#define IOFWD_TASKSM_BASETASKSM_HH

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "iofwdutil/tools.hh"
#include "zoidfs/util/ZoidFSAsync.hh"

namespace iofwd
{
    namespace tasksm
    {

/*
 * Simple state machine for IOFWD tasks.
 * 1) decode input and wait
 * 2) run the IOFWD operation and wait
 * 3) reply to client and wait
 */
class BaseTaskSM : public sm::SimpleSM< BaseTaskSM >
{
    public:
        BaseTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api) :
           sm::SimpleSM<BaseTaskSM>(smm), api_(api), slots_(*this)
        {
        }

        virtual ~BaseTaskSM()
        {
        }

        void init(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::postDecodeInput);
        }

        virtual void postDecodeInput(iofwdevent::CBException e) = 0;

        void waitDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::postRunOp);
        }

        virtual void postRunOp(iofwdevent::CBException e) = 0;

        void waitRunOp(iofwdevent::CBException e)
        {
           e.check ();
            setNextMethod(&BaseTaskSM::postReply);
        }

        virtual void postReply(iofwdevent::CBException e) = 0;

        void waitReply(iofwdevent::CBException e)
        {
           e.check ();
            // done
        }

    protected:

        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        zoidfs::util::ZoidFSAsync * api_;
        sm::SimpleSlots<NUM_BASE_SLOTS, iofwd::tasksm::BaseTaskSM> slots_;
};

    }
}
#endif
