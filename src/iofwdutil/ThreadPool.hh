#ifndef __IOFWDUTIL_THREADPOOL_HH__
#define __IOFWDUTIL_THREADPOOL_HH__

#include <queue>
#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>

#include "iofwdutil/Singleton.hh"

/*
    generic thread pool for the entire app

    round-robin scheduling across queues w/ three
    distinct priorities (HIGH, MID, LOW). Thread safe.
*/

namespace iofwdutil
{

class ThreadPool : public Singleton< ThreadPool >
{
    public:

        /* work item priority */
        enum TPPrio {HIGH, MID, LOW};

        ThreadPool();
        ~ThreadPool();

        void start();

        template <typename T>
        void cleanupWrapper(boost::function< void() > work, T * workObj)
        {
            /* run the work */
            work();

            /* delete the work object */
            delete workObj;
        }

        /* add work to the thread pool from an object */
        /* only accepts work that takes no args and has a void return type */
        template <typename T>
        void addWorkUnit(T * workObj, void (T::*workFunc)(void), TPPrio prio, bool cleanup)
        {
            /* if the thread pool is supposed to cleanup once the work is done */
            if(cleanup)
            {
                if(prio == HIGH)
                {
                    boost::mutex::scoped_lock lock(high_prio_queue_mutex_);
                    boost::function<void ()> work = boost::bind(workFunc, workObj);
                    boost::function<void ()> f = boost::bind(&ThreadPool::cleanupWrapper<T>, this, work, workObj);
                    high_prio_queue_queue_.push(f);

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
                else if(prio == MID)
                {
                    boost::mutex::scoped_lock lock(mid_prio_queue_mutex_);
                    boost::function<void ()> work = boost::bind(workFunc, workObj);
                    boost::function<void ()> f = boost::bind(&ThreadPool::cleanupWrapper<T>, this, work, workObj);
                    mid_prio_queue_queue_.push(f);

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
                else if(prio == LOW)
                {
                    boost::mutex::scoped_lock lock(low_prio_queue_mutex_);
                    boost::function<void ()> work = boost::bind(workFunc, workObj);
                    boost::function<void ()> f = boost::bind(&ThreadPool::cleanupWrapper<T>, this, work, workObj);
                    low_prio_queue_queue_.push(f);

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
            }
            /* else, the work will not delete the work obj */
            else
            {
                if(prio == HIGH)
                {
                    boost::mutex::scoped_lock lock(high_prio_queue_mutex_);
                    high_prio_queue_queue_.push(boost::bind(workFunc, workObj));

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
                else if(prio == MID)
                {
                    boost::mutex::scoped_lock lock(mid_prio_queue_mutex_);
                    mid_prio_queue_queue_.push(boost::bind(workFunc, workObj));

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
                else if(prio == LOW)
                {
                    boost::mutex::scoped_lock lock(low_prio_queue_mutex_);
                    low_prio_queue_queue_.push(boost::bind(workFunc, workObj));

                    boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                    threadlock.unlock();
                    threadcond_.notify_one();
                }
            }
        }

        /* add work to the thread pool from a free function */
        /* only accepts work that takes no args and has a void return type */
        void addWorkUnit(void (*workFunc)(void), TPPrio prio);

        static void setMinThreadCount(int c);
        static int getMinThreadCount();
        static void setMaxThreadCount(int c);
        static int getMaxThreadCount();
    protected:

        /* the thread function... polls queues for work and waits for work if non avail */
        void run(int tid);

        /* execute the the thread work */
        void runThreadWork(int tid);

        /* create a thread for the thread pool and detach it */
        void createThread(int tid);

        /* check the work queue for possible work items and move the work item if one was found */
        bool checkPrioWorkQueue(std::queue< boost::function< void() > > & tp_work_queue, boost::mutex & tp_work_queue_mutex, int tid);

        /* work queues */
        /* for now, we use predefined priroities seperated by queue... */
        /* DO NOT EXECUTE WORK WITHIN THESE DATA STRUCTURES... ONLY ADD AND RM WORK UNITS TO MINIMIZE LOCK HOLD TIMES */
        std::queue< boost::function< void() > > high_prio_queue_queue_;
        boost::mutex high_prio_queue_mutex_;
        std::queue< boost::function< void() > > mid_prio_queue_queue_;
        boost::mutex mid_prio_queue_mutex_;
        std::queue< boost::function< void() > > low_prio_queue_queue_;
        boost::mutex low_prio_queue_mutex_;

        /* storage for the current thread work items */
        /* NO LOCKS FOR THESE DATA STRUCTURES */
        std::vector< boost::function< void() > > thread_work_items_;

        /* thread tracking / usage variables */
        int thread_count_;
        boost::mutex shutdown_lock_;
        static boost::mutex tp_setup_mutex_;
        static int max_thread_count_;
        static int min_thread_count_;
        bool shutdown_threads_;
        bool thread_pool_running_;
        boost::condition threadcond_;
        boost::mutex thread_cond_mutex_;
        boost::condition shutdown_cond_;
        boost::mutex shutdown_cond_mutex_;
        boost::condition jobs_done_cond_;
        boost::mutex jobs_done_cond_mutex_;
};

} /* namespace iofwdutil */

#endif /* __IOFWDUTIL_THREADPOOL_HH__ */
