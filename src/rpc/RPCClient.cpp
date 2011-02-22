#include "RPCClient.hh"
#include "RPCEncoder.hh"
#include "RPCHeader.hh"
#include "RPCException.hh"

#include "iofwdevent/CBException.hh"

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

using namespace iofwdevent;

namespace rpc
{
   //========================================================================


      void RPCClient::startWrite ()
      {
         con_.out->write (&write_ptr_, &write_size_,
               boost::bind (&RPCClient::writeReady, this, _1), 0);
      }

      void RPCClient::writeReady (const CBException & e)
      {
         e.check ();

         RPCEncoder enc (write_ptr_, write_size_);
         RPCHeader header;
         header.key = key_;
         process (enc, header);
         const size_t used = enc.getPos ();
         con_.out->rewindOutput (write_size_ - used,
               boost::bind (&RPCClient::rewindReady, this, _1));
      }

      void RPCClient::rewindReady (const CBException & e)
      {
         e.check ();

         // try to read response
         con_.in->read (&read_ptr_, &read_size_,
               boost::bind (&RPCClient::readReady, this, _1), 0);

         boost::mutex::scoped_lock l (lock_);
         out_ready_ = true;
         if (out_ready_cb_)
         {
            iofwdevent::CBType cb;
            cb.swap (out_ready_cb_);
            l.unlock ();
            cb (iofwdevent::CBException ());
         }
      }

      void RPCClient::readReady (const CBException & e)
      {
         e.check ();

         RPCResponse r;
         RPCDecoder dec (read_ptr_, read_size_);
         process (dec, r);

         if (r.status != RPCResponse::RPC_OK)
         {
            handleError (r.status);
            return;
         }

         // everything OK, rewind input
         con_.in->rewindInput (read_size_ - dec.getPos(),
               boost::bind (&RPCClient::readRewindReady, this, _1));
      }

      void RPCClient::handleError (uint32_t )
      {
         /*
         switch (errcode)
         {
            case RPCResponse::RPC_UNKNOWN:
               cb (UnknownRPCKeyException ());
               break;
            default:
               cb (RPCCommException ());
               break;
         }*/
      }

      void RPCClient::readRewindReady (const CBException & e)
      {
         e.check ();

         boost::mutex::scoped_lock l(lock_);
         in_ready_ = true;
         if (in_ready_cb_)
         {
            iofwdevent::CBType cb;
            cb.swap (in_ready_cb_);
            l.unlock ();
            cb (e);
         }


         // If we want compression, we would modify h->con here and wrap the
         // input and/or outputstream with a compression stream
      }

   RPCClient::RPCClient (const RPCKey & key, const net::Connection & con)
      : key_(key),
        con_ (con),
        in_ready_ (false)
   {
      // First obtain write space
      startWrite ();
   }

   void RPCClient::waitOutReady (const iofwdevent::CBType & cb)
   {
      boost::mutex::scoped_lock l (lock_);
      if (out_ready_)
      {
         l.unlock ();
         cb (iofwdevent::CBException ());
         return;
      }
      out_ready_cb_ = cb;
   }

   void RPCClient::waitInReady (const iofwdevent::CBType & cb)
   {
      boost::mutex::scoped_lock l (lock_);
      if (in_ready_)
      {
         l.unlock ();
         cb (iofwdevent::CBException ());
         return;
      }

      in_ready_cb_ = cb;
   }

   RPCClientHandle RPCClient::rpcConnect (const RPCKey & key,
           const net::Connection & con)
   {
      return RPCClientHandle (new RPCClient (key, con));
   }

   //========================================================================
}
