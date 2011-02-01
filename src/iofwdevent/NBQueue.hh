#ifndef IOFWDEVENT_NBQUEUE_HH
#define IOFWDEVENT_NBQUEUE_HH

#include "iofwdevent/CBType.hh"
#include <queue>

namespace iofwdevent
{
   //========================================================================

   /**
    * A non-blocking queue using iofwdevent callbacks...
    *
    * Never blocks on enqueue, will expand storage when needed
    *
    * Dequeue only blocks (i.e. doesn't call CB immediately) when the queue is
    * empty.
    *
    * Thread safe.
    */
   template <typename T>
   class NBQueue
   {
      public:
         
         void enqueue (const T & data, const iofwdevent::CBType & cb);

         /**
          * Dequeue; Calls CB when data was taken from the queue
          * In absence of concurrent calls, this is guaranteed to call cb
          * before returning if !empty ()
          */
         void dequeue (T & data, const iofwdevent::CBType & cb);

         size_t size () const
         {
            boost::mutex::scoped_lock l(lock_);
            return queue_.size();
         }

         bool empty () const
         {
            boost::mutex::scoped_lock l(lock_);
            return queue_.empty ();
         }

      protected:
         // This function is called when data becomes available and there are
         // blocked readers
         static void waitDone (T * dest, const CBType & usercb, 
               const T & value);

      protected:
         mutable boost::mutex lock_;

         std::queue<T> queue_;

         typedef boost::function<void (const T & value)> WaitFunc;
         std::queue<WaitFunc> waiters_;
   };

   //========================================================================

   template <typename T>
   void NBQueue<T>::enqueue (const T & value, const iofwdevent::CBType & cb)
   {
      {
         boost::mutex::scoped_lock l(lock_);

         if (!waiters_.empty ())
         {
            // We have dequeuers waiting: the queue must be empty right now.
            // Don't bother adding, immediately pass data to the dequeuer.
            WaitFunc waitcb;
            waitcb.swap (waiters_.front ());
            waiters_.pop ();
            l.unlock ();

            waitcb (value);
            if (cb)
               cb (CBException ());
            return;
         }

         queue_.push (value);
      }
      if (cb)
         cb (iofwdevent::CBException ());
   }

   template <typename T>
   void NBQueue<T>::waitDone (T * dest, const CBType & cb, const T & value)
   {
      *dest = value;
      if (cb)
         cb (CBException ());
   }

   template <typename T>
   void NBQueue<T>::dequeue (T & value, const CBType & cb)
   {
      {
         boost::mutex::scoped_lock l(lock_);
         if (queue_.empty ())
         {
            waiters_.push (boost::bind (&NBQueue<T>::waitDone,
                       &value, cb, _1));
            return;
         }
         value = queue_.front ();
         queue_.pop ();
      }
      if (cb)
         cb (CBException ());
   }

   //========================================================================
}

#endif
