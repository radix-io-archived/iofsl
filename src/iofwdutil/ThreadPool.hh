#ifndef IOFWDUTIL_THREADPOOL_HH
#define IOFWDUTIL_THREADPOOL_HH

#include <queue>
#include <vector>
#include <stack>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/barrier.hpp>

#include "iofwdutil/Singleton.hh"

#define CRAY_TP_CORE_MAX 12
#define USE_CRAY_TP

/*
    generic thread pool for the entire app

    round-robin scheduling across queues w/ three
    distinct priorities (HIGH, MID, LOW). Thread safe.
*/

namespace iofwdutil
{

class ThreadPool : public Singleton< ThreadPool >
{
    protected:
        class IOFWDThread;

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

        void submitWorkUnit(boost::function<void (void)> func, TPPrio prio)
        {
            boost::mutex::scoped_lock lock(queue_mutex_);
            if(prio == HIGH)
            {
#ifndef USE_CRAY_TP
                if(active_threads_ >= max_high_thread_count_)
#else
                if(high_active_threads_ >= max_high_thread_count_)
#endif
                {
                    prio_queue_.push(func);
                }
                else
                {
#ifndef USE_CRAY_TP
                    active_threads_++;
                    lock.unlock();
                    boost::thread t (func);
#else
                    high_active_threads_++;
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    lock.unlock();
                    wt->setPrio(prio);
                    wt->setWork(func);
#endif
                }
            }
            else
            {
#ifndef USE_CRAY_TP
                if(active_threads_ >= max_norm_thread_count_)
#else
                if(norm_active_threads_ >= max_norm_thread_count_)
#endif
                {
                    norm_queue_.push(func);
                }
                else
                {
#ifndef USE_CRAY_TP
                    active_threads_++;
                    lock.unlock();
                    boost::thread t (func);
#else
                    norm_active_threads_++;
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    lock.unlock();
                    wt->setPrio(prio);
                    wt->setWork(func);
#endif
                }
            }
        }

        void workUnitDone()
        {
#ifndef USE_CRAY_TP
            boost::mutex::scoped_lock lock(queue_mutex_);
            active_threads_--;
#endif
        }

        void startWorkUnit()
        {
#ifndef USE_CRAY_TP
            boost::mutex::scoped_lock lock(queue_mutex_);
#endif

            if(prio_queue_.size() > 0)
            {
#ifndef USE_CRAY_TP
                boost::function<void ()> func = prio_queue_.front();
                prio_queue_.pop(); 
                active_threads_++;
                lock.unlock();
                boost::thread t (func);
#else
                boost::function<void ()> func = prio_queue_.front();
                IOFWDThread * wt = tpgroup_.top();
                tpgroup_.pop();
                wt->setWork(func);
#endif
            }
            else if(norm_queue_.size() > 0)
            {
#ifndef USE_CRAY_TP
                boost::function<void ()> func = norm_queue_.front();
                norm_queue_.pop(); 
                active_threads_++;
                lock.unlock();
                boost::thread t (func);
#else
                boost::function<void ()> func = norm_queue_.front();
                IOFWDThread * wt = tpgroup_.top();
                tpgroup_.pop();
                wt->setWork(func);
#endif
            }
        }

        bool addThread(iofwdutil::ThreadPool::IOFWDThread * t)
        {
            bool morework = false;
            boost::mutex::scoped_lock lock(queue_mutex_);

            if(prio_queue_.size() == 0 && norm_queue_.size() == 0)
            {
                if(t->getPrio() == HIGH)
                {
                    high_active_threads_--;
                }
                else
                {
                    norm_active_threads_--;
                } 
                tpgroup_.push(t);
            }
            else
            {
                if(prio_queue_.size() > 0)
                {
                    /* if this thread is going to switch priority */
                    if(t->getPrio() != HIGH)
                    {
                        norm_active_threads_--;

                        /* make sure we aren't already limited */
                        if(max_high_thread_count_ == high_active_threads_)
                        {
                            tpgroup_.push(t);
                            return morework;
                        }
                        else
                        {
                            high_active_threads_++;
                        }
                    }
                    
                    morework = true;
                    boost::function<void ()> func = prio_queue_.front();
                    prio_queue_.pop();
                    lock.unlock();
                    t->setPrio(HIGH);
                    t->setWorkNL(func);
                }
                else if(norm_queue_.size() > 0)
                {
                    /* if this thread is going to switch priority */
                    if(t->getPrio() == HIGH)
                    {
                        high_active_threads_--;

                        /* make sure we aren't already limited */
                        if(max_norm_thread_count_ == norm_active_threads_)
                        {
                            tpgroup_.push(t);
                            return morework;
                        }
                        else
                        {
                            norm_active_threads_++;
                        }
                    }
                    
                    morework = true;
                    boost::function<void ()> func = norm_queue_.front();
                    norm_queue_.pop();
                    lock.unlock();
                    t->setPrio(LOW);
                    t->setWorkNL(func);
                }
            }
            return morework;
        }

        /* add work to the thread pool from a free function */
        /* only accepts work that takes no args and has a void return type */
        void addWorkUnit(void (*workFunc)(void), TPPrio prio);

        static void setMinHighThreadCount(int c);
        static int getMinHighThreadCount();
        static void setMaxHighThreadCount(int c);
        static int getMaxHighThreadCount();
        static void setMinNormThreadCount(int c);
        static int getMinNormThreadCount();
        static void setMaxNormThreadCount(int c);
        static int getMaxNormThreadCount();

        void reset();
    protected:

class IOFWDThread
{
    public:
        IOFWDThread(ThreadPool * tp) : b_(2), shutdown_(false), tp_(tp), prio_(LOW)
        {
            noop_ = boost::function<void ()>(boost::bind(&IOFWDThread::noop, this));
            w_ = noop_;
            t_ = boost::thread(boost::bind(&IOFWDThread::operator(), this));
        }

        void setWork(boost::function<void ()> new_work)
        {
            boost::mutex::scoped_lock lock(mutex_);
            w_ = new_work;
            b_.wait();
        }

        void setWorkNL(boost::function<void ()> new_work)
        {
            w_ = new_work;
        }

        void kill()
        {
            shutdown_ = true;
            b_.wait();
        }

        void setPrio(TPPrio p)
        {
            prio_ = p;
        }
    
        TPPrio getPrio()
        {
            return prio_;
        }
    
    protected:
        /* the thread work function */
        void operator()()
        {
           bool morework = false;

#ifdef HAVE_LIBPTHREAD
           /* set the affinity of the thread */
           int j;
           cpu_set_t cpuset;
           pthread_t thread;

           /* get the thread id */
           thread = pthread_self();

           /* clear the cpu set */
           CPU_ZERO(&cpuset);

           /* set the number of cores to use to CRAY_TP_CORE_MAX */
           for (j = 0; j < CRAY_TP_CORE_MAX ; j++)
           {
               CPU_SET(j, &cpuset);
           }

           /* update the affinity */
           pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#endif

            while(!shutdown_)
            {
                /* wait for work */
                if(!morework)
                    b_.wait();

                if(shutdown_)
                    break;

                /* run some work */
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    w_();
                    w_ = noop_;
                }
                morework = tp_->addThread(this);
            }
        }

        void noop()
        {
        }

        boost::thread t_;
        boost::barrier b_;
        bool shutdown_;
        boost::mutex mutex_;
        ThreadPool * tp_;
        boost::function<void ()> w_; 
        boost::function<void ()> noop_;
        TPPrio prio_;
};

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
        int active_threads_;
        int high_active_threads_;
        int norm_active_threads_;
        std::queue< boost::function< void() > > prio_queue_;
        std::queue< boost::function< void() > > norm_queue_;
        boost::mutex queue_mutex_;
        std::queue< boost::function< void() > > high_prio_queue_queue_;
        boost::mutex high_prio_queue_mutex_;
        std::queue< boost::function< void() > > mid_prio_queue_queue_;
        boost::mutex mid_prio_queue_mutex_;
        std::queue< boost::function< void() > > low_prio_queue_queue_;
        boost::mutex low_prio_queue_mutex_;

        std::stack< IOFWDThread * > tpgroup_;

        /* storage for the current thread work items */
        /* NO LOCKS FOR THESE DATA STRUCTURES */
        std::vector< boost::function< void() > > thread_work_items_;

        /* thread tracking / usage variables */
        int thread_count_;
        boost::mutex shutdown_lock_;
        static boost::mutex tp_setup_mutex_;
        static int max_high_thread_count_;
        static int min_high_thread_count_;
        static int max_norm_thread_count_;
        static int min_norm_thread_count_;
        bool shutdown_threads_;
        bool thread_pool_running_;
        boost::condition threadcond_;
        boost::mutex thread_cond_mutex_;
        boost::condition shutdown_cond_;
        boost::mutex shutdown_cond_mutex_;
        boost::condition jobs_done_cond_;
        boost::mutex jobs_done_cond_mutex_;

        std::vector<boost::thread *> thread_vec_;

        boost::mutex tp_start_mutex_;
        bool started_;
        int tp_start_ref_count_;
};

class ThreadPoolKick
{
    public:
        ThreadPoolKick(iofwdutil::ThreadPool & tp) : tp_(tp)
        {
        }

        void operator()() const
        {
#ifndef USE_CRAY_TP
           tp_.workUnitDone();
           tp_.startWorkUnit();
#endif
        }


    protected:
        iofwdutil::ThreadPool & tp_;
};

} /* namespace iofwdutil */

#endif /* __IOFWDUTIL_THREADPOOL_HH__ */
