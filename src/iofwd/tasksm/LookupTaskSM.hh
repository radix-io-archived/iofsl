#ifndef __IOFWD_TASKSM_LOOKUPTASKSM_HH__
#define __IOFWD_TASKSM_LOOKUPTASKSM_HH__

#include "iofwd/tasksm/BaseTaskSM.hh"
#include "iofwdutil/InjectPool.hh"
#include "iofwd/LookupRequest.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace tasksm
    {

class LookupTaskSM : public BaseTaskSM, public iofwdutil::InjectPool< LookupTaskSM >
{
    public:
        LookupTaskSM (sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api, Request * request)
            : BaseTaskSM(smm, api), ret_(0), request_(static_cast<LookupRequest &>(*request))
        {
        }

        virtual ~LookupTaskSM()
        {
            delete &request_;
        }

        virtual void postDecodeInput(int UNUSED(status))
        {
            p_ = request_.decodeParam();
            setNextMethod(&BaseTaskSM::waitDecodeInput);
        }

        virtual void postRunOp(int UNUSED(status))
        {
            api_->lookup(slots_[BASE_SLOT], &ret_, p_.parent_handle, p_.component_name, p_.full_path, &handle_, p_.op_hint);
            slots_.wait(BASE_SLOT, &LookupTaskSM::waitRunOp);
        }

        virtual void postReply(int UNUSED(status))
        {
            request_.setReturnCode (ret_);
            request_.reply((slots_[BASE_SLOT]), (ret_  == zoidfs::ZFS_OK ? &handle_ : 0));
            slots_.wait(BASE_SLOT, &LookupTaskSM::waitReply);
        }

    protected:
        int ret_;
        LookupRequest & request_;
        LookupRequest::ReqParam p_;
        zoidfs::zoidfs_handle_t handle_;
};

    }
}
#endif
