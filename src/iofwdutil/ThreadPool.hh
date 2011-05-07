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
#include <boost/algorithm/string.hpp>

#include "iofwdutil/Singleton.hh"

#include <iostream>

#define CRAY_TP_CORE_MAX 12

/*
    generic thread pool for the entire app

    round-robin scheduling across queues w/ three
    distinct priorities (HIGH, MID, LOW). Thread safe.
*/

#define TPStringify( name ) # name

namespace iofwdutil
{

class ThreadPool : public Singleton< ThreadPool >
{
    protected:
        class IOFWDThread;

        typedef std::queue< boost::function< void() > > IOFWDThreadStorage;

    public:

        /* work item priority */
        enum TPPrio {HIGH=0, LOW, PRIO_LIMIT};

        /* work item attribute */
        enum TPAttr {FILEIO=0, SM, RPC, OTHER, ATTR_LIMIT};

        static TPAttr getAttrType(std::string attrString)
        {
            if(boost::equals(attrString, std::string(TPStringify( FILEIO ))))
            {
                return FILEIO;
            }
            else if(boost::equals(attrString, std::string(TPStringify( SM ))))
            {
                return RPC;
            }
            else if(boost::equals(attrString, std::string(TPStringify( RPC ))))
            {
                return SM;
            }
            return OTHER;
        }

        static std::string getAttrString(TPAttr a)
        {
            switch(a)
            {
                case FILEIO:
                    return std::string("FILEIO");
                case SM:
                    return std::string("SM");
                case RPC:
                    return std::string("RPC");
                default:
                    return std::string("OTHER");
            };
            return std::string("OTHER");
        }

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

        void submitWorkUnit(boost::function<void (void)> func, TPPrio prio,
                TPAttr attr)
        {
            boost::mutex::scoped_lock lock(queue_mutex_);
            if(prio == HIGH)
            {
                if(high_active_threads_[attr] >= max_high_thread_count_[attr])
                {
                    prio_queue_[attr].push(func);
                }
                else
                {
                    high_active_threads_[attr]++;
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    lock.unlock();
                    wt->setPrio(prio);
                    wt->setAttr(attr);
                    wt->setWork(func);
                }
            }
            else
            {
                if(norm_active_threads_[attr] >= max_norm_thread_count_[attr])
                {
                    norm_queue_[attr].push(func);
                }
                else
                {
                    norm_active_threads_[attr]++;
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    lock.unlock();
                    wt->setPrio(prio);
                    wt->setAttr(attr);
                    wt->setWork(func);
                }
            }
        }

        void startWorkUnit()
        {
            /* check high prio */
            for(int i = FILEIO ; i < ATTR_LIMIT ; i++)
            {
                if(prio_queue_[i].size() > 0)
                {
                    boost::function<void ()> func = prio_queue_[i].front();
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    wt->setAttr(static_cast<TPAttr>(i));
                    wt->setWork(func);

                    break;
                }
            }
            for(int i = FILEIO ; i < ATTR_LIMIT ; i++)
            {
                if(norm_queue_[i].size() > 0)
                {
                    boost::function<void ()> func = norm_queue_[i].front();
                    IOFWDThread * wt = tpgroup_.top();
                    tpgroup_.pop();
                    wt->setAttr(static_cast<TPAttr>(i));
                    wt->setWork(func);

                    break;
                }
            }
        }

        bool addThread(iofwdutil::ThreadPool::IOFWDThread * t)
        {
            bool morework = false;
            boost::mutex::scoped_lock lock(queue_mutex_);
            int reusethread = 0;

            //for(int i = 0 ; i < ATTR_LIMIT ; i++)
            {
                if(prio_queue_[t->getAttr()].size() != 0 ||
                        norm_queue_[t->getAttr()].size() != 0)
                {
                    reusethread = 1;
                }
            }

            if(!reusethread)
            {
                if(t->getPrio() == HIGH)
                {
                    high_active_threads_[t->getAttr()]--;
                }
                else if(t->getPrio() == LOW)
                {
                    norm_active_threads_[t->getAttr()]--;
                } 
                tpgroup_.push(t);
            }
            else
            {
                {
                    if(prio_queue_[t->getAttr()].size() > 0)
                    {
                        /* if this thread is going to switch priority */
                        if(t->getPrio() != HIGH)
                        {
                            norm_active_threads_[t->getAttr()]--;

                            /* make sure we aren't already limited */
                            if(max_high_thread_count_[t->getAttr()] ==
                                    high_active_threads_[t->getAttr()])
                            {
                                tpgroup_.push(t);
                                return morework;
                            }
                            else
                            {
                                high_active_threads_[t->getAttr()]++;
                            }
                        }
                    
                        morework = true;
                        boost::function<void ()> func = prio_queue_[t->getAttr()].front();
                        prio_queue_[t->getAttr()].pop();
                        lock.unlock();
                        t->setPrio(HIGH);
                        t->setWorkNL(func);
                    }
                    else if(norm_queue_[t->getAttr()].size() > 0)
                    {
                        /* if this thread is going to switch priority */
                        if(t->getPrio() == HIGH)
                        {
                            high_active_threads_[t->getAttr()]--;

                            /* make sure we aren't already limited */
                            if(max_norm_thread_count_[t->getAttr()] ==
                                    norm_active_threads_[t->getAttr()])
                            {
                                tpgroup_.push(t);
                                return morework;
                            }
                            else
                            {
                                norm_active_threads_[t->getAttr()]++;
                            }
                        }
                    
                        morework = true;
                        boost::function<void ()> func = norm_queue_[t->getAttr()].front();
                        norm_queue_[t->getAttr()].pop();
                        lock.unlock();
                        t->setPrio(LOW);
                        t->setWorkNL(func);
                    }
                }
            }
            return morework;
        }

        /* TP config */
        static void setMaxHighThreadCount(TPAttr a, int c);
        static int getMaxHighThreadCount(TPAttr a);
        static void setMaxNormThreadCount(TPAttr a, int c);
        static int getMaxNormThreadCount(TPAttr a);
        static void setNormThreadWarn(TPAttr a, int c);
        static void setHighThreadWarn(TPAttr a, int c);

        void reset();
    protected:

class IOFWDThread
{
    public:
        IOFWDThread(ThreadPool * tp) :
            b_(2),
            shutdown_(false),
            tp_(tp),
            prio_(LOW),
            attr_(OTHER)
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

        void setAttr(TPAttr a)
        {
            attr_ = a;
        }
    
        TPPrio getPrio()
        {
            return prio_;
        }

        TPAttr getAttr()
        {
            return attr_;
        }

        void noop()
        {
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

        boost::thread t_;
        boost::barrier b_;
        bool shutdown_;
        boost::mutex mutex_;
        ThreadPool * tp_;
        boost::function<void ()> w_; 
        boost::function<void ()> noop_;
        TPPrio prio_;
        TPAttr attr_;
};

        /* work queues */
        std::map<int, int> high_active_threads_;
        std::map<int, int> norm_active_threads_;
        std::map<int, IOFWDThreadStorage> prio_queue_;
        std::map<int, IOFWDThreadStorage> norm_queue_;
        boost::mutex queue_mutex_;

        std::stack< IOFWDThread * > tpgroup_;

        /* thread tracking / usage variables */
        boost::mutex shutdown_lock_;
        static boost::mutex tp_setup_mutex_;
        static std::map<int, int> max_high_thread_count_;
        static std::map<int, int> high_thread_warn_;
        static std::map<int, int> max_norm_thread_count_;
        static std::map<int, int> norm_thread_warn_;
        bool shutdown_threads_;
        bool thread_pool_running_;

        boost::mutex tp_start_mutex_;
        bool started_;
};

} /* namespace iofwdutil */

#endif /* __IOFWDUTIL_THREADPOOL_HH__ */
