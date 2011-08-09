#ifndef SRC_IOFWDUTIL_SELFPIPE_HH
#define SRC_IOFWDUTIL_SELFPIPE_HH

#include <boost/thread/mutex.hpp>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cstdio>

#include <boost/thread/condition_variable.hpp>

//#define IOFWD_SP_USE_PIPES 1

namespace iofwdutil
{

class SelfPipe
{
    public:
        SelfPipe(void) :
            ssig_(0),
            nsig_(0),
            fdmax_(0),
            cond_waiting_(false),
            notify_count_(0),
            cond_wait_sig_(false)
        {
#ifdef IOFWD_SP_USE_PIPES
            pipe(fd_);
            FD_ZERO(&fdset_);
            FD_SET(fd_[0], &fdset_);

            if(fd_[0] > fdmax_)
            {
                fdmax_ = fd_[0];
            }
#endif
        }

        ~SelfPipe(void);

        bool startSignal(void)
        {
            {
                boost::mutex::scoped_lock lock(ssig_mutex_);
                ssig_++;
            }
            return true;
        }

        size_t resetStartSignalTotal(void)
        {
            size_t ssig = 0;

            {
                boost::mutex::scoped_lock lock(ssig_mutex_);
                ssig = ssig_;
                ssig_ = 0;
            }

            return ssig;
        }

        bool doneSignal(void)
        {
            char c = 'w';
            int ret = 0;

            {
#ifdef IOFWD_SP_USE_PIPES
                boost::mutex::scoped_lock lock1(nsig_mutex_);
                boost::mutex::scoped_lock lock2(fd_mutex_);
                nsig_++;
                ret = write(fd_[1], &c, 1);
#else
                {
                    boost::mutex::scoped_lock lock2(nsig_mutex_);
                    nsig_++;
                }

                {
                    boost::lock_guard<boost::mutex> lock1(cond_wait_mutex_);
                    
                    if(cond_waiting_ )
                    {
                        if(cond_wait_sig_)
                        {
                            notify_count_++;
                        }
                        else
                        {
                            cond_wait_sig_ = true;
                            cond_.notify_one();
                        }
                    }
                    else
                    {
                        notify_count_++;
                    }
                }

#endif
            }
            if(ret != 1)
                return false;
            return true;
        }

        void wait(size_t n)
        {
#ifdef IOFWD_SP_USE_PIPES
            size_t nsig = 0;
            int ret = 0;
            char buffer[1024];
            size_t waitfor = n;
            struct timespec deadline;

            while(waitfor > 0)
            {
                deadline.tv_sec = 1;
                deadline.tv_nsec = 0;

                ret = pselect(fdmax_ + 1, &fdset_, NULL, NULL, &deadline,
                        NULL);

                {
                    boost::mutex::scoped_lock lock1(nsig_mutex_);
                    nsig = nsig_;
                }

                if(ret > 0)
                {
                    /* lock  */

                    if(FD_ISSET(fd_[0], &fdset_))
                    {
                        if(waitfor > nsig)
                        {
                            while(nsig > 0)
                            {
                                size_t b = 0;
                                if(nsig > 1024)
                                {
                                    b = 1024;
                                }
                                else
                                {
                                    b = nsig;
                                }

                                {
                                    boost::mutex::scoped_lock lock2(fd_mutex_);
                                    read(fd_[0], buffer, b);
                                }

                                waitfor -= b;
                                nsig -= b;
                            }
                        }
                        else
                        {
                            while(waitfor > 0)
                            {
                                size_t b = 0;
                                if(waitfor > 1024)
                                {
                                    b = 1024;
                                }
                                else
                                {
                                    b = waitfor;
                                }

                                {
                                    boost::mutex::scoped_lock lock2(fd_mutex_);
                                    read(fd_[0], buffer, b);
                                }

                                waitfor -= b;
                                nsig -= b;
                            }
                        }
                    }
                }

                /* clear the fd */
                reset();

                /* unlock */
            }

            if(nsig > 0)
            {
                boost::mutex::scoped_lock lock1(nsig_mutex_);
                nsig_ += nsig;
            }
#else
            {
                boost::unique_lock<boost::mutex> lock1(cond_wait_mutex_);
                size_t waitfor = n;
                {
                    waitfor -= notify_count_;
                    notify_count_ = 0;
                }

                /* wait for pending signals */
                while(waitfor > 0)
                {
                    cond_waiting_ = true;
                    cond_wait_sig_ = false;
                    cond_.wait(lock1);
                
                    waitfor--;
                    waitfor-= notify_count_;
                    notify_count_ = 0;
                }
            
                cond_waiting_ = false;
                cond_wait_sig_ = false;
            }

            {
                boost::mutex::scoped_lock lock2(nsig_mutex_);
                nsig_--;
            }
#endif
        }

    protected:
        void reset(void)
        {
#ifdef IOFWD_SP_USE_PIPES
            FD_ZERO(&fdset_);
            FD_SET(fd_[0], &fdset_);
#endif
        }

        int fd_[2];
        boost::mutex fd_mutex_;
        fd_set fdset_;
        
        size_t ssig_;
        boost::mutex ssig_mutex_;

        size_t nsig_;
        boost::mutex nsig_mutex_;

        int fdmax_;

        boost::mutex cond_wait_mutex_;
        bool cond_waiting_;
        size_t notify_count_;
        bool cond_wait_sig_;

        boost::condition_variable cond_;
};

}

#endif
