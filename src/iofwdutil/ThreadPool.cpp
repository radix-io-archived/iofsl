#include "iofwdutil/ThreadPool.hh"

namespace iofwdutil
{
        int iofwdutil::ThreadPool::min_high_thread_count_ = 1;
        int iofwdutil::ThreadPool::max_high_thread_count_ = 1;
        int iofwdutil::ThreadPool::min_norm_thread_count_ = 1;
        int iofwdutil::ThreadPool::max_norm_thread_count_ = 1;

        boost::mutex iofwdutil::ThreadPool::tp_setup_mutex_;

        void ThreadPool::setMaxHighThreadCount(int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            max_high_thread_count_ = c;
        }

        void ThreadPool::setMaxNormThreadCount(int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            max_norm_thread_count_ = c;
        }

        void ThreadPool::setMinHighThreadCount(int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            min_high_thread_count_ = c;
        }

        void ThreadPool::setMinNormThreadCount(int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            min_norm_thread_count_ = c;
        }

        int ThreadPool::getMinNormThreadCount()
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return min_norm_thread_count_;
        }

        int ThreadPool::getMinHighThreadCount()
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return min_high_thread_count_;
        }

        int ThreadPool::getMaxNormThreadCount()
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return max_norm_thread_count_;
        }

        int ThreadPool::getMaxHighThreadCount()
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return max_high_thread_count_;
        }

        void ThreadPool::reset()
        {
            /* mark as shutting down */
            {
                boost::mutex::scoped_lock lock(tp_start_mutex_);

                /* if it had not been started, do not try to shutdown */
                if(!started_)
                    return;
                else
                {
                    /* dec the ref count */
                    tp_start_ref_count_--;

                    /* if the ref count i 0, shutdown */
                    if(tp_start_ref_count_ == 0)
                    {
                        started_ = false;
                    }
                    /* if not 0, somebody else is using the tp... do not shutdown */
                    else
                    {
                        return;
                    }
                }
            }

            {
                boost::mutex::scoped_lock lock(shutdown_cond_mutex_);

                /* shutdown the active threads */
                shutdown_threads_ = true;
            }

            /* while there are still active threads */
            threadcond_.notify_all();
           
            /* join all of the dyn alloc'ed threads and delete them */ 
            for(unsigned int i = 0 ; i < thread_vec_.size() ; i++)
            {
                thread_vec_[i]->join();
                delete (thread_vec_[i]);
            }

            /* remove all of the elements from the vector */
            thread_vec_.clear();
        }

        ThreadPool::ThreadPool() : active_threads_(0), thread_count_(0), shutdown_threads_(false), started_(false), tp_start_ref_count_(0)
        {
        }

        void ThreadPool::start()
        {
            /* check and see if the tp is started */
            {
                boost::mutex::scoped_lock lock(tp_start_mutex_);

                tp_start_ref_count_++;
                if(started_)
                    return;
                else
                    started_ = true;
            }

            thread_count_ = 0;

            /* setup the storage queue for thread work */
            thread_work_items_.resize(max_norm_thread_count_);
        }

        ThreadPool::~ThreadPool()
        {
            reset();
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
                {
                    if(shutdown_threads_)
                    {
                        thread_count_--;
                        return;
                    }
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
            thread_vec_.push_back(new boost::thread(boost::bind(&ThreadPool::run, this, tid)));
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
