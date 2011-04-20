#ifndef IOFWD_TASKSM_RESIZETASKSM_HH
#define IOFWD_TASKSM_RESIZETASKSM_HH

#include "iofwdutil/InjectPool.hh"
#include "iofwd/ResizeRequest.hh"
#include "zoidfs/zoidfs.h"

#include "iofwdutil/IOFSLKeyValueStorage.hh"
#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "zoidfs/util/ZoidFSAsync.hh"

namespace iofwd
{
    namespace tasksm
    {

class ResizeTaskSM : public sm::SimpleSM< ResizeTaskSM >, 
                     public iofwdutil::InjectPool< ResizeTaskSM >
{
    public:
        ResizeTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : sm::SimpleSM<ResizeTaskSM>(smm),
              ret_(0),
              request_(static_cast<ResizeRequest &>(*request)),
              api_(api),
              slots_(*this)
        {
        }

        virtual ~ResizeTaskSM()
        {
            delete &request_;
        }

        void init(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&ResizeTaskSM::postDecodeInput);
        }

        void waitDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&ResizeTaskSM::postRunOp);
        }

        void waitRunOp(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&ResizeTaskSM::postResetAtomicAppendOffset);
        }

        void waitReply(iofwdevent::CBException e)
        {
            e.check ();
            // done
        }

        virtual void waitResetAtomicAppendOffset(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&ResizeTaskSM::postReply);
        }

        virtual void postResetAtomicAppendOffset(iofwdevent::CBException e)
        {
            e.check ();

            {
                iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();
                zoidfs::zoidfs_file_ofs_t aa_offset =
                    static_cast<zoidfs::zoidfs_file_ofs_t>(p_.size);

                key.setFileHandle(p_.handle);
                key.setDataKey(std::string("NEXTAPPENDOFFSET"));

                iofwdutil::IOFSLKeyValueStorage::instance().initKeyValue< 
                    zoidfs::zoidfs_file_ofs_t >(slots_[BASE_SLOT], key,
                    aa_offset);

                slots_.wait(BASE_SLOT,
                    &ResizeTaskSM::waitResetAtomicAppendOffset);
            }
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
           e.check ();
            p_ = request_.decodeParam();
            setNextMethod(&ResizeTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
           e.check ();
            api_->resize(slots_[BASE_SLOT], &ret_, p_.handle, p_.size,
                    (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &ResizeTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
           e.check ();
            request_.setReturnCode(ret_);
            request_.reply((slots_[BASE_SLOT]));
            slots_.wait(BASE_SLOT, &ResizeTaskSM::waitReply);
        }

    protected:
        int ret_;
        ResizeRequest & request_;
        ResizeRequest::ReqParam p_;

        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        zoidfs::util::ZoidFSAsync * api_;
        sm::SimpleSlots<NUM_BASE_SLOTS, iofwd::tasksm::ResizeTaskSM> slots_;
};

    }
}
#endif
