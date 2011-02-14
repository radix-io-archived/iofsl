#include "LoopbackInput.hh"
#include "iofwdutil/assert.hh"

#include "iofwdutil/tools.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      LoopbackInput::LoopbackInput (MessageQueuePtr ptr)
         : queue_ (ptr), get_ptr_(0), get_size_(0),
         rewindptr_(0), rewindsize_(0), rewindused_(0)
      {
      }

      iofwdevent::Handle LoopbackInput::read (const void ** ptr, size_t * size,
            const iofwdevent::CBType & cb, size_t UNUSED(suggested))
      {
         // have leftover, use that first
         if (rewindused_ < rewindsize_)
         {
            *ptr = static_cast<const char *>(rewindptr_) + rewindused_;
            *size = rewindsize_ - rewindused_;
            rewindused_ = rewindsize_;
            cb (iofwdevent::CBException ());
            return 0;
         }

         // Free old block
         delete [] (static_cast<const char*>(rewindptr_));

         rewindused_ = 0;
         get_ptr_ = ptr;
         get_size_ = size;
         cb_ = cb;
         queue_->get (const_cast<void **>(&rewindptr_), &rewindsize_,
               boost::bind (&LoopbackInput::getComplete, this, _1));
         return 0;
      }

      void LoopbackInput::getComplete (const iofwdevent::CBException & e)
      {
         e.check ();
         *get_ptr_ = rewindptr_;
         *get_size_ = rewindsize_;
         rewindused_ = rewindsize_;

         // in case the somebody calls read from the callback
         iofwdevent::CBType cb;
         cb.swap (cb_);
         cb (iofwdevent::CBException ());
      }

      iofwdevent::Handle LoopbackInput::rewindInput (size_t size,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(rewindused_ >= size);
         rewindused_ -= size;
         cb (iofwdevent::CBException ());
         return 0;
      }

      LoopbackInput::~LoopbackInput ()
      {
         delete[] (static_cast<const char*> (rewindptr_));
      }

      //=====================================================================
   }
}
