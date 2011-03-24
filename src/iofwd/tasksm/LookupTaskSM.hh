#ifndef IOFWD_TASKSM_LOOKUPTASKSM_HH
#define IOFWD_TASKSM_LOOKUPTASKSM_HH

#include "iofwdutil/InjectPool.hh"
#include "iofwd/LookupRequest.hh"
#include "zoidfs/zoidfs.h"

#include "iofwdutil/IOFSLKeyValueStorage.hh"
#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"
#include "zoidfs/util/ZoidFSAsync.hh"

#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
    namespace tasksm
    {

class LookupTaskSM : public sm::SimpleSM< LookupTaskSM >,
                     public iofwdutil::InjectPool< LookupTaskSM >
{
    public:
        LookupTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api,
              Request * request)
            : sm::SimpleSM<LookupTaskSM>(smm),
              ret_(0),
              request_(static_cast<LookupRequest &>(*request)),
              api_(api),
              slots_(*this)
        {
        }

        virtual ~LookupTaskSM()
        {
            delete &request_;
        }

        void init(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&LookupTaskSM::postDecodeInput);
        }

        void waitDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&LookupTaskSM::postRunOp);
        }

        void waitRunOp(iofwdevent::CBException e)
        {
            e.check ();
            //setNextMethod(&LookupTaskSM::postResetAtomicAppendOffset);
            setNextMethod(&LookupTaskSM::postReply);
        }

        void waitReply(iofwdevent::CBException e)
        {
            e.check ();
            // done
        }

        virtual void waitResetAtomicAppendOffset(iofwdevent::CBException e)
        {
            e.check ();
            setNextMethod(&LookupTaskSM::postReply);
        }

        virtual void postResetAtomicAppendOffset(iofwdevent::CBException e)
        {
            e.check ();
#if 0
           /* atomic append hint vars */
            int aavallen = 0;
            int aaflag = 0;

            /* check if the atomic append hint was set */
            zoidfs::hints::zoidfs_hint_get_valuelen(*(p_.op_hint),
                ZOIDFS_ATOMIC_APPEND_SEEK, &aavallen, &aaflag);

            if(aaflag)
            {
                char value[ZOIDFS_ATOMIC_APPEND_OFFSET_MAX_BYTES];
                iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();
                zoidfs::zoidfs_file_ofs_t new_offset = 0;

                /* extract the new offset from the hint */
                zoidfs::hints::zoidfs_hint_get(*(p_.op_hint), ZOIDFS_ATOMIC_APPEND_SEEK,
                    ZOIDFS_ATOMIC_APPEND_OFFSET_MAX_BYTES, value, &aaflag);
                new_offset = atol(value);


                key.setParentHandle(p_.parent_handle);
                if(p_.component_name)
                {
                    key.setComponentName(std::string(p_.component_name)); 
                }
                if(p_.full_path)
                {
                    key.setFilePath(std::string(p_.full_path));
                }
                key.setFileHandle(&handle_);
                key.setDataKey(std::string("NEXTAPPENDOFFSET"));

                iofwdutil::IOFSLKeyValueStorage::instance().initKeyValue< 
                    zoidfs::zoidfs_file_ofs_t >(slots_[BASE_SLOT], key,
                    new_offset);
                slots_.wait(BASE_SLOT,
                    &LookupTaskSM::waitResetAtomicAppendOffset);
            }
            else
#endif
            {
                setNextMethod(&LookupTaskSM::postReply);
            }
        }

        virtual void postDecodeInput(iofwdevent::CBException e)
        {
            e.check ();
            p_ = request_.decodeParam();
            setNextMethod(&LookupTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(iofwdevent::CBException e)
        {
            e.check ();
            api_->lookup(slots_[BASE_SLOT], &ret_, p_.parent_handle,
                  p_.component_name, p_.full_path, &handle_, p_.op_hint);
            slots_.wait(BASE_SLOT, &LookupTaskSM::waitRunOp);
        }

        virtual void postReply(iofwdevent::CBException e)
        {
            e.check ();
            request_.setReturnCode (ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ?
                     &handle_ : 0));
            slots_.wait(BASE_SLOT, &LookupTaskSM::waitReply);
        }

    protected:
        int ret_;
        LookupRequest & request_;
        LookupRequest::ReqParam p_;
        zoidfs::zoidfs_handle_t handle_;

        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        zoidfs::util::ZoidFSAsync * api_;
        sm::SimpleSlots<NUM_BASE_SLOTS, iofwd::tasksm::LookupTaskSM> slots_;
};

    }
}
#endif
