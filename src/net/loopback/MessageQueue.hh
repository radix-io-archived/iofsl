#ifndef NET_LOOPBACK_MESSAGEQUEUE_HH
#define NET_LOOPBACK_MESSAGEQUEUE_HH

#include "iofwdevent/CBType.hh"
#include "iofwdutil/IntrusiveHelper.hh"
#include "iofwdevent/NBQueue.hh"

#include <boost/intrusive_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>


namespace net
{
   namespace loopback
   {
      //=====================================================================

      class MessageQueue : public iofwdutil::IntrusiveHelper
      {
         public:

            /// Put takes ownership of the buffer.
            /// Assumes type is char[]
            void put (void * ptr, size_t size);

            /// Transfers ownership back to caller
            void get (void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb);

            /// For cleaning up unconsumed messages
            ~MessageQueue ();


         protected:

            typedef boost::tuple<void *, size_t> Entry;
            iofwdevent::NBQueue<Entry> queue_;

            typedef boost::tuple<void **, size_t *, Entry, iofwdevent::CBType>
               GetWaitData;

         protected:
            static void getDone (GetWaitData * w, const
                  iofwdevent::CBException & e);

      };

      INTRUSIVE_PTR_HELPER(MessageQueue);

      typedef boost::intrusive_ptr<MessageQueue> MessageQueuePtr;

      //=====================================================================
   }
}
#endif
