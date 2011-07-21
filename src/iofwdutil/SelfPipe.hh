#ifndef SRC_IOFWDUTIL_SELFPIPE_HH
#define SRC_IOFWDUTIL_SELFPIPE_HH

#include <boost/thread/mutex.hpp>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

namespace iofwdutil
{

class SelfPipe
{
    public:
        SelfPipe(void) :
            ssig_(0),
            nsig_(0),
            fdmax_(0)
        {
            pipe(fd_);
            FD_ZERO(&fdset_);
            FD_SET(fd_[0], &fdset_);

            if(fd_[0] > fdmax_)
            {
                fdmax_ = fd_[0];
            }
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
                boost::mutex::scoped_lock lock1(nsig_mutex_);
                boost::mutex::scoped_lock lock2(fd_mutex_);
                nsig_++;
                ret = write(fd_[1], &c, 1);
            }
            if(ret != 1)
                return false;
            return true;
        }

        void wait(size_t n)
        {
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
        }

    protected:
        void reset(void)
        {
            FD_ZERO(&fdset_);
            FD_SET(fd_[0], &fdset_);
        }

        int fd_[2];
        boost::mutex fd_mutex_;
        fd_set fdset_;
        
        size_t ssig_;
        boost::mutex ssig_mutex_;

        size_t nsig_;
        boost::mutex nsig_mutex_;

        int fdmax_;
};

}

#endif
