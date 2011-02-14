#include "LoopbackConnector.hh"

#include "LoopbackAddress.hh"
#include "LoopbackInput.hh"
#include "LoopbackOutput.hh"
#include "MessageQueue.hh"

#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      LoopbackConnector::LoopbackConnector ()
         : localhost_ (new LoopbackAddress ())
      {
      }

      LoopbackConnector::~LoopbackConnector ()
      {
      }

      void LoopbackConnector::setAcceptHandler (
            const AcceptHandler & h)
      {
         boost::mutex::scoped_lock l(lock_);
         accept_ = h;
      }

      void LoopbackConnector::lookup (const char * location,
            AddressPtr * ptr,
            const iofwdevent::CBType & cb)
      {
         ALWAYS_ASSERT(location);
         ALWAYS_ASSERT(ptr);
         *ptr = localhost_;
         cb (iofwdevent::CBException ());
         return;
      }

      /**
       * Create two MessageQueues, basically two unidirectional pipes, one for
       * reading from the remote and one for sending to the remote
       */
      Connection LoopbackConnector::connect (const AddressPtr & )
      {
         MessageQueuePtr chan1 (new MessageQueue ());
         MessageQueuePtr chan2 (new MessageQueue ());

         newQuery (new LoopbackInput (chan1),
               new LoopbackOutput (chan2, DEFAULT_BLOCKSIZE, MAX_BLOCKSIZE));

         return Connection (
               new LoopbackInput (chan2),
               new LoopbackOutput (chan1, DEFAULT_BLOCKSIZE, MAX_BLOCKSIZE));
      }

      void LoopbackConnector::newQuery (ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out)
      {
         // We assume that the RPCExec callback will not try to create a new
         // commchanncel (which would deadlock here)
         boost::mutex::scoped_lock l(lock_);

         if (!accept_)
         {
            l.unlock ();
            delete (in);
            delete (out);
            return;
         }

         AcceptInfo info;
         info.source = localhost_;
         info.in = in;
         info.out = out;

         accept_ (info);
      }
            
      void LoopbackConnector::createGroup (GroupHandle * UNUSED(group),
               const std::vector<std::string> & UNUSED(members),
               const iofwdevent::CBType & UNUSED(cb))
      {
         ALWAYS_ASSERT(false);
      }

      //=====================================================================
   }
}
