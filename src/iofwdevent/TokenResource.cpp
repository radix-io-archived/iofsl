#include "TokenResource.hh"

#include "iofwdutil/assert.hh"

#include <boost/utility.hpp>


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

   /**
    * @TODO: review this: address might be reused, use sequence nr to avoid
    * this.
    */
   bool TokenResource::cancel (Handle h)
   {
      TokenRequest * found = 0;
      {
         boost::mutex::scoped_lock l (lock_);
         RequestList::iterator I = waitlist_.begin ();

         while (I != waitlist_.end ())
         {
            if (boost::addressof (*I) == h)
            {
               // Found operation
               found = &(*I);
               waitlist_.erase (I);
               break;
            }
         }
         if (!found)
            return false;
      }

      CBType cb;
      cb.swap (found->cb_);
      delete (found);

      // Call the callback with Cancelled exception
      cb (CBException::cancelledOperation (h));

      return true;
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

   bool TokenResource::notify_next (iofwdevent::CBType & cb)
   {
      if (waitlist_.empty () || (waitlist_.front().tokens_ > tokens_available_))
         return false;

      ASSERT (!waitlist_.empty ());
      TokenRequest * f = &waitlist_.front ();

      // tokens_ shouldn't be zero (because such a request would never be
      // queued)
      ALWAYS_ASSERT(f->tokens_);

      ALWAYS_ASSERT(tokens_available_ >= f->tokens_);
      tokens_available_ -= f->tokens_;
      cb = f->cb_;
      waitlist_.pop_front ();
      delete (f);
      return true;
   }


   void TokenResource::add_tokens (size_t tokens)
   {
      return add_tokens_limit (tokens, 0);
   }

   /**
    * Add tokens but don't let the amount of free tokens go over limit.
    * Note: when using add_tokens_limit, requests for more than limit will never
    * complete.
    */
   void TokenResource::add_tokens_limit (size_t tokens, size_t limit)
   {
      if (!tokens)
         return;

      iofwdevent::CBType cb;
      boost::mutex::scoped_lock l(lock_);
      if (limit)
      {
         if (tokens_available_ >= limit)
            return;
         tokens_available_ += std::min (limit - tokens_available_, tokens);
      }
      else
      {
         tokens_available_ += tokens;
      }

      if (check (cb))
      {
         l.unlock ();
         cb (CBException ());
      }
   }

   Resource::Handle TokenResource::request (const CBType & cb, size_t tokencount)
   {
      ASSERT(started_);

      boost::mutex::scoped_lock l(lock_);

      // Cannot call try_request; already have lock
      if (try_request_unlocked (tokencount))
      {
         // Unlock before calling callback, so if the callback requests more
         // tokens we don't deadlock.
         l.unlock ();
         // obtained tokens -> call callback
         cb (CBException ());
         return 0;
      }

      // Could not get tokens -> put on waitlist
      TokenRequest * f = new  TokenRequest (cb, tokencount);
      waitlist_.push_back (*f);
      return f;
   }

   bool TokenResource::try_request (size_t t)
   {
      ASSERT(started_);

      boost::mutex::scoped_lock l(lock_);
      return try_request_unlocked (t);
   }

   bool TokenResource::try_request_unlocked (size_t tokencount)
   {
      if (!tokencount)
         return true;

      /* if there is a waitlist, always queue to keep fairness */
      if (!waitlist_.empty ())
         return false;

      // if tokencount == 0, the request always succeeds
      if (tokens_available_ >= tokencount)
      {
         tokens_available_ -= tokencount;
         return true;
      }

      return false;
   }

   void TokenResource::release (size_t tokencount)
   {
      CBType t;
      boost::mutex::scoped_lock l (lock_);

      ASSERT(started_);

      tokens_available_ += tokencount;
      if (check (t))
      {
         l.unlock ();
         t (CBException ());
      }
   }

   bool TokenResource::check (iofwdevent::CBType & t)
   {
      if (waitlist_.empty())
         return false;

      if (waitlist_.front().tokens_ <= tokens_available_)
         return notify_next (t);

      return false;
   }

   //===========================================================================
}
