#include "iofwd/TaskPoolHelper.hh"

namespace iofwd
{
    void resetTaskHelper(ThreadTaskParam & param)
    {
         /* cleanup old request */
         if(full_init_)
         {
            delete (request_);
         }

         /* this is a full init */
         full_init_ = true;

         /* set new request params */
         request_ = static_cast<T *>(param.req);
         api_ = param.api;
         async_api_ = param.async_api;
         sched_ = param.sched;
         bpool_ = param.bpool;
    }

    virtual void cleanup()
    {
         int opid = request_->getOpID();
         if(full_init_)
         {
             delete request_;
             request_ = NULL;
             full_init_ = false;
         }
         tpool_->put(opid, this);
    }
}
