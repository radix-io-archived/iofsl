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
        IOCBWrapper(iofwdevent::CBType cb, int count, int * ret)
            : cb_(cb), count_(count), ret_(ret), isRead(false)
        {
        }

        void invoke(int status)
        {
            boost::mutex::scoped_lock l(lock_);
            
            /* if this is the last callback ref, issue the callback */
            if(count_ == 1)
            {
                /* set the ret code */
                *(ret_) = status;

                /* do copies if this is a read op */
                if(isRead)
                    copy();

                /* invoke the callback */
                cb_(status);
                
                /* unlock */
                l.unlock();

                /* destroy this object */
                delete this;
            }
            /* else, dec the ref count */
            else
            {
                count_--;
            }
        }

        /* add a copy request to the cb wrapper */
        void addCopy(char * dbuf, size_t d_off, char * obuf, size_t o_off, size_t size)
        {
           copy_wrappers_.push_back(IOCBCopyWrapper(dbuf, d_off, obuf, o_off, size)); 
        }

        iofwdevent::CBType cb_;
        int count_;
        boost::mutex lock_;
        int * ret_;
        bool isRead;

    protected:

        /* wrapper for the copy operations for consumed buffers */
        class IOCBCopyWrapper
        {
            public:
                IOCBCopyWrapper(char * d, size_t df, char * o, size_t of, size_t s)
                    : dbuf_(d), doffset_(df), obuf_(o), ooffset_(of), size_(s)
                {
                }

                char * dbuf_;
                size_t doffset_;
                char * obuf_;
                size_t ooffset_;
                size_t size_;
        };

        std::vector<IOCBCopyWrapper> copy_wrappers_;

        /* copy data collected from this operation into consumed buffers that share this range */
        void copy()
        {
            /* run each copy */
            for(int i = 0 ; i < copy_wrappers_.size() ; i++)
            {
                /* validate that we have non-null buffers */
                if(copy_wrappers_[i].dbuf_ && copy_wrappers_[i].obuf_)
                {
                    memcpy(copy_wrappers_[i].dbuf_ + copy_wrappers_[i].doffset_, copy_wrappers_[i].obuf_ + copy_wrappers_[i].ooffset_, copy_wrappers_[i].size_);
                }
            }
            /* remove all of the copy requests */
            copy_wrappers_.clear();
        }

        ~IOCBWrapper();
};
    }
}

#endif
