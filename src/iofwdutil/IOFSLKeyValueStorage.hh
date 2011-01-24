#ifndef IOFWDUTIL_IOFSLKEYVALUESTORAGE_HH
#define IOFWDUTIL_IOFSLKEYVALUESTORAGE_HH

#include <map>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "iofwdutil/Singleton.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdevent/CBException.hh"

namespace iofwdutil
{

/*
 * Key / value storage for IOFSL
 *
 * Currently, this is just accessible by a single IOFSL server
 *
 */
class IOFSLKeyValueStorage : public Singleton< IOFSLKeyValueStorage >
{
    public:
        IOFSLKeyValueStorage();
        ~IOFSLKeyValueStorage();

        void lockTable()
        {
        }

        void unlockTable()
        {
        }

        void process(iofwdevent::CBType cb, boost::function<void(void)> wu) 
        {
                boost::mutex::scoped_lock rll(request_list_mutex_);

                if(executeRequest(wu, rll))
                {
                    cb(iofwdevent::CBException());
                    return;
                }

                /* could not access the table */
                KVRequest * k = new KVRequest(cb, wu); 
                pending_kv_requests_.push_back(*k);

                return;
        }

        void progress()
        {
            boost::mutex::scoped_lock rll(request_list_mutex_);
            if(pending_kv_requests_.empty())
            {
                /* push work unit onto thread pool */
            }
        }

        bool executeRequest(boost::function<void(void)> wu, boost::mutex::scoped_lock & rll)
        {
                /* allow fifo processing of requests */
                if(!pending_kv_requests_.empty())
                {
                    return false;
                }

                boost::mutex::scoped_try_lock tl(table_mutex_);
                if(tl.owns_lock())
                {
                    /* remove the lock on the kv request list */
                    rll.unlock();

                    /* update the KV */
                    wu();

                    return true;
                }

                return false;
        }

        template <typename T>
        void fetchAndInc(iofwdevent::CBType cb, std::string key, T nextValueInc, T * curValue=NULL)
        {
            /* atomic fetch and inc of the value */
            {
                boost::mutex::scoped_lock l(table_mutex_);

                /* try to get the table lock... */
                {
                    /* if the value is not in the table, create it */
                    if(table_.find(key) == table_.end())
                    {
                        T * val = new T();
                        *val = nextValueInc;
                        if(curValue)
                        {
                            *curValue = 0; /* init to 0 */
                        }

                        table_[key] = val;
                    }
                    /* else fetch and update the value */
                    else
                    {
                        T * tval = static_cast<T *>(table_[key]);
                        if(curValue)
                        {
                            *curValue = *tval;
                        }
                        *tval += nextValueInc;
                    }
                }
            }

            /* invoke the callback */
            cb(iofwdevent::CBException());
        }

        template <typename T>
        void fetchAndDrop(iofwdevent::CBType cb, std::string key, T * curValue=NULL)
        {
            /* atomic drop of the value */
            {
                boost::mutex::scoped_lock l(table_mutex_);

                {
                    /* value not in the table... */
                    if(table_.find(key) == table_.end())
                    {
                        if(curValue)
                        {
                            *curValue = 0; /* set to 0 */
                        }
                    }
                    /* else fetch and drop the value */
                    else
                    {
                        T * tval = static_cast<T *>(table_[key]);
                        if(curValue)
                        {
                            *curValue = *tval;
                        }
                        table_.erase(key);
                        delete tval;
                    }
                }
            }

            /* invoke the callback */
            cb(iofwdevent::CBException());
        }

        void reset()
        {
            /* purge all key / value pairs */
        }

    protected:

        class KVRequest
        {
            public:
                KVRequest (const iofwdevent::CBType & cb, boost::function<void(void)> wu)
                    : cb_(cb), wu_(wu)
                {
                }

                iofwdevent::CBType cb_;
                boost::function<void(void)> wu_;

                boost::intrusive::slist_member_hook<> list_hook_;
        }; 

        typedef boost::intrusive::member_hook<KVRequest, boost::intrusive::slist_member_hook<>, &KVRequest::list_hook_ > KVMemberHook;
        typedef boost::intrusive::slist<KVRequest, KVMemberHook, boost::intrusive::cache_last<true>  > KVRequestList;
        
        boost::mutex table_mutex_;
        std::map< std::string, void * > table_;

        KVRequestList pending_kv_requests_; 
        boost::mutex request_list_mutex_;
};

}

#endif
