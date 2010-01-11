#ifndef IOFWDEVENT_TOKENRESOURCE_HH
#define IOFWDEVENT_TOKENRESOURCE_HH

#include <boost/pool/pool_alloc.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/thread.hpp>
#include "iofwdutil/assert.hh"
#include "Resource.hh"

namespace iofwdevent
{
//===========================================================================

/**
 * This resource limits the amount of 'tokens' in use; It will block
 * requests for more until enough tokens have been returned.
 *
 * It can be used to limit the number of outstanding transactions, memory, ...
 *
 * The resource does not use an internal thread and does not need to be
 * polled.
 *
 * Currently does FIFO order.
 */
class TokenResource : public Resource
{
public:
   TokenResource (size_t tokens = 0);

   ~TokenResource ();

   virtual void start ();

   virtual void stop ();

   virtual bool started () const;

   virtual bool cancel (Handle h);

   /**
    * Submit request for tokencount tokens.
    */
   inline Handle request (const CBType & cb, size_t tokencount);

   /**
    * Non-blocking token request. Returns true if allocated, false
    * otherwise.
    */
   inline bool try_request (size_t tokencount);

   /**
    * Return tokens obtain through request
    */
   inline void release (size_t tokencount);

   /** 
    * Add more tokens to the system; Can be used for a rate-limiting scheme.
    */
   inline void add_tokens (size_t tokencount);

   /**
    * Return number of free tokens
    */
   inline size_t get_tokencount () const
   { return tokens_available_; }

protected:
   

protected:
   // Note: Although constructor/destructor is allowed,
   // destructors will only be called when no exceptions occur.
   // This could be fixed by having the TokenRequest destructor destruct all
   // the entries in the waitlist_
   class TokenRequest
   {
    public:
       TokenRequest (const CBType & cb, size_t tokens)
          : cb_(cb), tokens_(tokens)
       {
       }

      CBType   cb_;
      size_t   tokens_;

      // we use an intrusive linked list to queue the requests.
      boost::intrusive::slist_member_hook<> list_hook_;
   } ;

   typedef boost::intrusive::member_hook<TokenRequest, boost::intrusive::slist_member_hook<>,
           &TokenRequest::list_hook_ > MemberOption;

   // cache_last is needed for push_back
   typedef boost::intrusive::slist<TokenRequest, MemberOption, boost::intrusive::cache_last<true>  > RequestList;

protected:

   // Needs to be called with lock held
   inline bool try_request_unlocked (size_t tokens);

   // Do a quick check to see if we need to notify a waiting client
   //  Needs to be called with lock held
   inline void check ();

   // Service next client; Needs to be called with lock held
   void notify_next ();

protected:
   // Protect access to waitresource
   mutable boost::mutex lock_;

   size_t tokens_available_;

   RequestList waitlist_;

   bool started_;
};

//===========================================================================

void TokenResource::add_tokens (size_t tokens)
{
   boost::mutex::scoped_lock l(lock_);
   tokens_available_ += tokens;
   check ();
}

Resource::Handle TokenResource::request (const CBType & cb, size_t tokencount)
{
   ASSERT(started_);

   boost::mutex::scoped_lock l(lock_);

   // Cannot call try_request; already have lock
   if (try_request_unlocked (tokencount))
   {
      // obtained tokens -> call callback
      cb (COMPLETED);
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
   ASSERT(started_);

   boost::mutex::scoped_lock l (lock_);

   tokens_available_ += tokencount;
   check ();
}

void TokenResource::check ()
{
   if (waitlist_.empty())
      return;

   if (waitlist_.front().tokens_ <= tokens_available_)
      notify_next ();
}

//===========================================================================
}

#endif
