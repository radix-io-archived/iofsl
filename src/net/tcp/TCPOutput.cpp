#include "TCPOutput.hh"
#include "iofwdutil/assert.hh"

namespace net
{
    namespace tcp
    {
        TCPOutput::TCPOutput (boost::shared_ptr<TCPConnection> connection,
                size_t def_blocksize,
                size_t max_blocksize) :
            def_blocksize_(def_blocksize),
            max_blocksize_(max_blocksize),
            thisblock_ptr_(new char[4096]),
            thisblock_size_(4096),
            thisblock_used_(0),
            connection_(connection)
        {
        }
        
        TCPOutput::~TCPOutput()
        {
            std::cout << __func__ << std::endl;
            delete [] (static_cast<const char*> (thisblock_ptr_));
        }
        
        iofwdevent::Handle TCPOutput::flush(const iofwdevent::CBType & cb)
        {
            internal_flush(cb);

            return 0;
        }

        iofwdevent::Handle TCPOutput::write(void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t suggested)
        {
            // If we have some leftover space, use that first
            if(thisblock_used_ < thisblock_size_)
            {
                *ptr = static_cast<char *> (thisblock_ptr_) + thisblock_used_;
                *size = thisblock_size_ - thisblock_used_;

                // Mark everything we gave out as used
                thisblock_used_ += *size;

                cb(iofwdevent::CBException());
                
                return 0;
            }

            internal_flush(cb);

            const size_t newblocksize = suggested ? 
                std::min(max_blocksize_, suggested) : def_blocksize_;

            // we need to get a new block
            thisblock_ptr_ = new char [newblocksize];
            thisblock_size_ = newblocksize;
            thisblock_used_ = 0;

            *ptr = thisblock_ptr_;
            *size = thisblock_size_;

            return 0;
        }

        iofwdevent::Handle TCPOutput::rewindOutput(size_t size,
            const iofwdevent::CBType & cb)
        {
            ALWAYS_ASSERT(size <= thisblock_used_);
            thisblock_used_ -= size;
            cb(iofwdevent::CBException());

            return 0;
        }

        void TCPOutput::internal_flush(const iofwdevent::CBType & cb)
        {
            void * blockPtr = NULL;
            size_t blockSize = 0;
            size_t blockUsed = 0;

            if(!thisblock_ptr_)
                return;

            /* temp copy of the buffer data */
            blockPtr = thisblock_ptr_;
            blockSize = thisblock_size_;
            blockUsed = thisblock_used_;

            /* reset the buffer data */
            thisblock_ptr_ = 0;
            thisblock_size_ = 0;
            thisblock_used_ = 0;

            /* post the send */
            connection_->post_send(boost::bind(&TCPOutput::internal_flush_done,
                        this, blockPtr, cb, _1),
                    blockPtr, blockUsed);
        }

        void TCPOutput::internal_flush_done(void * buffer, const
                iofwdevent::CBType & cb, iofwdevent::CBException e)
        {
            delete [] (static_cast<const char*>(buffer));
            cb(e);
        }
    }
}
