#include "iofwdutil/ThreadPool.hh"

namespace iofwdutil
{
        std::map<int, int> iofwdutil::ThreadPool::max_high_thread_count_;
        std::map<int, int> iofwdutil::ThreadPool::max_norm_thread_count_;
        std::map<int, int> iofwdutil::ThreadPool::norm_thread_warn_;
        std::map<int, int> iofwdutil::ThreadPool::high_thread_warn_;

        boost::mutex iofwdutil::ThreadPool::tp_setup_mutex_;
        
        void ThreadPool::setNormThreadWarn(TPAttr a, int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            norm_thread_warn_[a] = c;
        }

        void ThreadPool::setHighThreadWarn(TPAttr a, int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            high_thread_warn_[a] = c;
        }

        void ThreadPool::setMaxHighThreadCount(TPAttr a, int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            max_high_thread_count_[a] = c;
        }

        void ThreadPool::setMaxNormThreadCount(TPAttr a, int c)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            max_norm_thread_count_[a] = c;
        }

        int ThreadPool::getMaxNormThreadCount(TPAttr a)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return max_norm_thread_count_[a];
        }

        int ThreadPool::getMaxHighThreadCount(TPAttr a)
        {
            boost::mutex::scoped_lock lock(tp_setup_mutex_);
            return max_high_thread_count_[a];
        }


        ThreadPool::ThreadPool() :
            shutdown_threads_(false),
            started_(false)
        {
            for(int i = FILEIO ; i < ATTR_LIMIT ; i++)
            {
                high_active_threads_[static_cast<TPAttr>(i)] = 0;
                norm_active_threads_[static_cast<TPAttr>(i)] = 0;
            } 
        }

        void ThreadPool::start()
        {
            /* check and see if the tp is started */
            {
                boost::mutex::scoped_lock lock(tp_start_mutex_);

                if(started_)
                    return;
                else
                    started_ = true;
            }

            for(int a = 0 ; a < ATTR_LIMIT ; a++)
            {
                for(int i = 0 ; i < max_high_thread_count_[a] +
                    max_norm_thread_count_[a] ; i++)
                {
                    tpgroup_.push(new IOFWDThread(this));
                }
            }
        }

        ThreadPool::~ThreadPool()
        {
        }
}
