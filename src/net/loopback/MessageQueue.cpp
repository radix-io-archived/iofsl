#include "iofwdutil/assert.hh"

#include "MessageQueue.hh"

#include <boost/scoped_ptr.hpp>

namespace net
{
   namespace loopback
   {
      //======================================================================

      MessageQueue::~MessageQueue ()
      {
         if (queue_.empty ())
            return;

         // Need to cleanup unconsumed messages
         // We know the CB will be called immediately, since we're the only
         // ones accessing queue
         while (!queue_.empty ())
         {
            Entry e;
            queue_.dequeue (e, iofwdevent::CBType ());
            delete [] (static_cast<char*>(e.get<0>()));
         }
      }


      void MessageQueue::put (void * ptr, size_t size)
      {
         // We assume enqueue is never blocking (as specified by NBQueue
         // semantics)
         queue_.enqueue (boost::make_tuple(ptr, size), iofwdevent::CBType ());
      }

      void MessageQueue::getDone (GetWaitData * w,
            const iofwdevent::CBException & e)
      {
         boost::scoped_ptr<GetWaitData> ptr (w);

         e.check ();

         const Entry & data = w->get<2>();
         const iofwdevent::CBType & cb = w->get<3>();

         *(w->get<0>()) = data.get<0>();
         *(w->get<1>()) = data.get<1>();

         cb (e);
      }

      void MessageQueue::get (void ** ptr, size_t * size,
            const iofwdevent::CBType & cb)
      {
         GetWaitData * w = new GetWaitData (ptr, size, Entry(), cb);
         queue_.dequeue (w->get<2>(),
               boost::bind (&MessageQueue::getDone, w, _1));
      }

      //======================================================================
   }
}
