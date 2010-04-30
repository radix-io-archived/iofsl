#ifndef __IOFWD_TASKSM_IOCBWRAPPER_HH__
#define __IOFWD_TASKSM_IOCBWRAPPER_HH__

#include "iofwdevent/CBType.hh"
#include <boost/thread/mutex.hpp>

namespace iofwd
{
    namespace tasksm
    {

/*
 * Used when multiple callbacks are mapped to one
 * Ref count the callbacks and issue the parent callback when the last
 *  child callback is issued
 */
class IOCBWrapper
{
    public:
        IOCBWrapper(iofwdevent::CBType cb, int count)
            : cb_(cb), count_(count)
        {
        }

        void invoke(int status)
        {
            boost::mutex::scoped_lock l(lock_);
            if(count_ == 1)
            {
                cb_(status);
                l.unlock();
                delete this;
            }
            else
            {
                count_--;
            }
        }

        iofwdevent::CBType cb_;
        int count_;
        boost::mutex lock_;

    protected:
        ~IOCBWrapper();
};
    }
}

#endif
