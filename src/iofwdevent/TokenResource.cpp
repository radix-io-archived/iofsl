#include "TokenResource.hh"

namespace iofwdevent
{
//===========================================================================

   TokenResource::TokenResource (size_t tokens)
      : tokens_available_ (tokens), started_ (false)
   {
   }

   TokenResource::~TokenResource ()
   {
   }

   void TokenResource::start ()
   {
      boost::mutex::scoped_lock l (lock_);

      ALWAYS_ASSERT(!started_);
      // don't need to do anything; no thread
      started_ = true;
   }

   void TokenResource::stop ()
   {
      boost::mutex::scoped_lock l (lock_);

      ALWAYS_ASSERT(started_);
      started_ = false;
   }

   bool TokenResource::started () const
   {
      boost::mutex::scoped_lock l (lock_);

      return started_;
   }

   void TokenResource::notify_next ()
   {
      if (waitlist_.empty () || (waitlist_.front().tokens_ > tokens_available_))
         return;

      ASSERT (!waitlist_.empty ());
      TokenRequest * f = &waitlist_.front ();

      // tokens_ shouldn't be zero (because such a request would never be
      // queued)
      ALWAYS_ASSERT(f->tokens_);

      ALWAYS_ASSERT(tokens_available_ >= f->tokens_);
      tokens_available_ -= f->tokens_;
      f->op_->success ();

      waitlist_.pop_front ();

      // We call the destructor here just in case, but note that if an
      // exception occurs allocated objects will *not* be destroyed.
      f->~TokenRequest ();

      // Note this is protected by lock_
      pool_.deallocate (f);
   }

//===========================================================================
}
