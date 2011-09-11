#ifndef IOFWD_TASKSM_REMOVETASKSM_HH
#define IOFWD_TASKSM_REMOVETASKSM_HH

#include "iofwdutil/InjectPool.hh"
#include "iofwd/RemoveRequest.hh"
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

class RemoveTaskSM : public sm::SimpleSM< RemoveTaskSM >,
                     public iofwdutil::InjectPool< RemoveTaskSM >
{
    public:
        RemoveTaskSM (Request * request, const SharedData & shared)
            : sm::SimpleSM<RemoveTaskSM>(shared.smm),
              ret_(0),
              request_(static_cast<RemoveRequest *>(request)),
              api_(shared.api),
              slots_(*this)
        {
            ZOIDFS_CACHE_HINT_INIT(hint_);
        }

        void init(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&RemoveTaskSM::postDecodeInput);
        }

        void waitDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&RemoveTaskSM::postDropAtomicAppendOffset);
        }

        void waitRunOp(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&RemoveTaskSM::postReply);
        }

        void waitReply(iofwdevent::CBException e)
        {
            e.check ();
            // done
        }

        virtual void waitDropAtomicAppendOffset(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&RemoveTaskSM::postRunOp);
        }

        virtual void postDropAtomicAppendOffset(iofwdevent::CBException e)
        {
#if 1
            iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();

            key.setParentHandle(p_.parent_handle);
            if(p_.component_name)
            {
                key.setComponentName(std::string(p_.component_name));
            }
            if(p_.full_path)
            {
                key.setFilePath(std::string(p_.full_path));
            }
            key.setDataKey(std::string("NEXTAPPENDOFFSET"));

            e.check ();

            iofwdutil::IOFSLKeyValueStorage::instance().fetchAndDrop<zoidfs::zoidfs_file_ofs_t>(slots_[BASE_SLOT], key);
            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitDropAtomicAppendOffset);
#else
            /* issue the rpc using the nonblocking aa manager */
            iofwd::extraservice::AtomicAppendManager::instance().issueDeleteOffsetRPC(
                slots_[BASE_SLOT],
                p_.handle,
                &atomic_append_base_offset_);

            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitDropAtomicAppendOffset);
#endif
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            p_ = request_->decodeParam();
            setNextMethod(&RemoveTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
            e.check ();
            api_->remove(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name,
                               p_.full_path, &hint_, (*p_.op_hint)());
            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
            e.check ();
            request_->setReturnCode(ret_);
            request_->reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? &hint_ : 0));
            slots_.wait(BASE_SLOT, &RemoveTaskSM::waitReply);
        }

    protected:
        int ret_;
        boost::scoped_ptr<RemoveRequest> request_;
        RemoveRequest::ReqParam p_;
        zoidfs::zoidfs_cache_hint_t hint_;
        
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        zoidfs::util::ZoidFSAsync * api_;
        sm::SimpleSlots<NUM_BASE_SLOTS, iofwd::tasksm::RemoveTaskSM> slots_;
};

    }
}
#endif
