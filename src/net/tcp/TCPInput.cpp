#include "TCPInput.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

namespace net
{
    namespace tcp
    {
        TCPInput::TCPInput(boost::shared_ptr<TCPConnection> connection) :
            get_ptr_(0),
            get_size_(0),
            rewindptr_(0),
            rewindsize_(0),
            rewindused_(0),
            max_packet_size_(4096),
            connection_(connection),
            data_ready_(false)
        {
        }

        iofwdevent::Handle TCPInput::read(const void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t UNUSED(suggested))
        {
            // have leftover, use that first
            if (rewindused_ < rewindsize_ && rewindptr_ != NULL && data_ready_)
            {
                *ptr = static_cast<const char *>(rewindptr_) + rewindused_;
                *size = rewindsize_ - rewindused_;
                rewindused_ = rewindsize_;
                cb(iofwdevent::CBException ());
                return 0;
            }

            /* free the old buffer */
            data_ready_ = false;
            delete [] (static_cast<const char*>(rewindptr_));
            rewindptr_ = new char[max_packet_size_];
            rewindsize_ = max_packet_size_;
            rewindused_ = 0;

            /* store user params */
            get_ptr_ = ptr;
            get_size_ = size;

            /* post a recv */
            connection_->post_recv(boost::bind(&TCPInput::readComplete, this,
                        cb, _1),
                    const_cast<void *>(rewindptr_),
                    &rewindsize_);

            return 0;
        }

        void TCPInput::readComplete(const iofwdevent::CBType & cb,
                const iofwdevent::CBException & e)
        {
            e.check();
            *get_ptr_ = rewindptr_;
            *get_size_ = rewindsize_;
            rewindused_ = rewindsize_;

            data_ready_ = true;
            cb(iofwdevent::CBException());
        }

        iofwdevent::Handle TCPInput::rewindInput(size_t size,
            const iofwdevent::CBType & cb)
        {
            ALWAYS_ASSERT(rewindused_ >= size);
            rewindused_ -= size;
            cb(iofwdevent::CBException ());
            
            return 0;
        }

        TCPInput::~TCPInput()
        {  
            std::cout << __func__ << std::endl; 
            delete[] (static_cast<const char*> (rewindptr_));
        }
    }
}
