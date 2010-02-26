#include "iofwdutil/ThreadPool.hh"

namespace iofwdutil
{
        boost::once_flag iofwdutil::ThreadPool::start_flag_ = BOOST_ONCE_INIT;

        ThreadPool::ThreadPool() : thread_count_(0),
            max_thread_count_(0), min_thread_count_(0), shutdown_threads_(false)
        {
        }

        void ThreadPool::start(int min_thread_count, int max_thread_count)
        {
            /* only call this once */
            boost::function<void ()> f1 = boost::bind(&ThreadPool::start_internal, this, min_thread_count, max_thread_count);
            boost::call_once(start_flag_, f1);
        }

        void ThreadPool::start_internal(int min_thread_count, int max_thread_count)
        {
            min_thread_count_ = min_thread_count;
            max_thread_count_ = max_thread_count;
            thread_count_ = 0;

            /* setup the storage queue for thread work */
            thread_work_items_.resize(max_thread_count_);

            /* create the max number of threads */
            for(thread_count_ = 0 ; thread_count_ < max_thread_count_ ; thread_count_++)
            {
                createThread(thread_count_);
            }
        }

        ThreadPool::~ThreadPool()
        {
            boost::mutex::scoped_lock lock(shutdown_cond_mutex_);

            /* shutdown the active threads */
            shutdown_threads_ = true;

            /* while there are still active threads */
            threadcond_.notify_all();
            while(thread_count_ > 0)
            {
                if(thread_count_ > 0)
                {
                    shutdown_cond_.wait(lock);
                }
            }
        }

        /* add work to the thread pool from a free function */
        /* only accepts work that takes no args and has a void return type */
        void ThreadPool::addWorkUnit(void (*workFunc)(void), TPPrio prio)
        {
            if(prio == HIGH)
            {
                boost::mutex::scoped_lock lock(high_prio_queue_mutex_);
                high_prio_queue_queue_.push(boost::function< void() >(workFunc));

                boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                threadlock.unlock();
                threadcond_.notify_one();
            }
            else if(prio == MID)
            {
                boost::mutex::scoped_lock lock(mid_prio_queue_mutex_);
                mid_prio_queue_queue_.push(boost::function< void() >(workFunc));

                boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                threadlock.unlock();
                threadcond_.notify_one();
            }
            else if(prio == LOW)
            {
                boost::mutex::scoped_lock lock(low_prio_queue_mutex_);
                low_prio_queue_queue_.push(boost::function< void() >(workFunc));

                boost::mutex::scoped_lock threadlock(thread_cond_mutex_);
                threadlock.unlock();
                threadcond_.notify_one();
            }
        }

        /* the thread function... polls queues for work and waits for work if non avail */
        void ThreadPool::run(int tid)
        {
            bool didWork = false;

            /* wait for some work before the do / while is entered */
            do
            {
                /* check the queues for work */
                if(checkPrioWorkQueue(high_prio_queue_queue_, high_prio_queue_mutex_, tid))
                {
                    runThreadWork(tid);
                    didWork = true;
                }
                else if(checkPrioWorkQueue(mid_prio_queue_queue_, mid_prio_queue_mutex_, tid))
                {
                    runThreadWork(tid);
                    didWork = true;
                }
                else if(checkPrioWorkQueue(low_prio_queue_queue_, low_prio_queue_mutex_, tid))
                {
                    runThreadWork(tid);
                    didWork = true;
                }

                /* wait for work */
                if(!didWork && !shutdown_threads_)
                {
                    boost::mutex::scoped_lock lock(thread_cond_mutex_);
                    threadcond_.wait(lock);
                }

                /* reset the work flag */
                didWork = false;

                /* if shutdown flag was set, exit the function */
                if(shutdown_threads_)
                {
                    thread_count_--;

                    if(thread_count_ == 0)
                    {
                        shutdown_cond_.notify_one();
                    }
                    return;
                }
            }while(true);
        }

        /* execute the the thread work */
        void ThreadPool::runThreadWork(int tid)
        {
            thread_work_items_[tid]();
            thread_work_items_[tid] = 0;
        }

        /* create a thread for the thread pool and detach it */
        void ThreadPool::createThread(int tid)
        {
            boost::thread t (boost::bind(&ThreadPool::run, this, tid));

            t.detach();
        }

        /* check the work queue for possible work items and move the work item if one was found */
        bool ThreadPool::checkPrioWorkQueue(std::queue< boost::function< void() > > & tp_work_queue, boost::mutex & tp_work_queue_mutex, int tid)
        {
            boost::mutex::scoped_lock lock(tp_work_queue_mutex);

            if(tp_work_queue.size() > 0)
            {
                thread_work_items_[tid] = tp_work_queue.front();
                tp_work_queue.pop();
                return true;
            }
            return false;
        }
}
