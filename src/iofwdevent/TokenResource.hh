#ifndef IOFWDEVENT_TOKENRESOURCE_HH
#define IOFWDEVENT_TOKENRESOURCE_HH

#include <boost/pool/pool_alloc.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/thread.hpp>
#include "Resource.hh"
#include "CBType.hh"

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
 * polled. As such, it can be created and stored inside the classes that want
 * to use it.
 *
 * Currently does FIFO order; Locks are released before calling the callback,
 * so concurrent callbacks are possible. If multiple threads are used, and
 * some threads are adding tokens when there are multiple threads requesting
 * tokens, a strict FIFO order can no longer be guaranteed.
 *
 */
class TokenResource : public Resource
{
public:
   TokenResource (size_t tokens = 0, size_t tokens_warn = 0, std::string
           id=std::string(""));

   ~TokenResource ();

   virtual void start ();

   virtual void stop ();

   virtual bool started () const;

   /**
    * Cancel request for tokens.
    *
    * Will return false if the request already completed.
    * If the request can be cancelled, the callback will be called with an
    * exception indication cancellation.
    */
   virtual bool cancel (Handle h);

   /**
    * Submit request for tokencount tokens.
    */
   Handle request (const CBType & cb, size_t tokencount);

   /**
    * Non-blocking token request. Returns true if allocated, false
    * otherwise.
    */
   bool try_request (size_t tokencount);

   /**
    * Return tokens obtained through try_request or request.
    */
   void release (size_t tokencount);

   /** 
    * Add more tokens to the system; Can be used for a rate-limiting scheme.
    */
   void add_tokens (size_t tokencount);

   /**
    * Add more tokens to the system, but make sure the total number of tokens
    * doesn't exceed limit.
    */
   void add_tokens_limit (size_t tokencount, size_t limit);

   /**
    * Return number of free tokens
    *
    * NOTE: This function is for debugging/testing only, since it is
    * inherently prone to race conditions. By the time the function returns,
    * the number of tokens could have changed, so there is no guarantee a
    * subsequent request for tokens will not block.
    *
    * Use try_request instead.
    */
   inline size_t get_tokencount () const;

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
   bool try_request_unlocked (size_t tokens);

   // Do a quick check to see if we need to notify a waiting client
   //  Needs to be called with lock held
   //  Sets the callback if one can be completed
   bool check (iofwdevent::CBType & t);

   // Service next client; Needs to be called with lock held
   bool notify_next (iofwdevent::CBType & t);

protected:
   // Protect access to waitresource
   mutable boost::mutex lock_;

   size_t tokens_available_;
   size_t tokens_available_warn_thresh_;

   RequestList waitlist_;

   bool started_;

   std::string id_;
};

//===========================================================================

 size_t TokenResource::get_tokencount () const
 {
    return tokens_available_;
 }

//===========================================================================
}

#endif
