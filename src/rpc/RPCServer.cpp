#include "RPCServer.hh"

#include <boost/scoped_ptr.hpp>

#include "iofwdutil/IOFWDLog.hh"
#include "RPCRegistry.hh"
#include "RPCEncoder.hh"
#include "RPCHeader.hh"
#include "RPCException.hh"
#include "RPCInfo.hh"

#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"

namespace rpc
{
   //========================================================================

   class RPCServer::RPCHelper
   {
      public:
         enum State
         {
            INIT = 0,
            READ_READY,
            REWIND_READY,
            WRITE_READY,
            WRITE_REWIND_READY,
            DONE
         };

         RPCHelper (RPCServer & server, const net::Net::AcceptInfo & info)
            : server_ (server), info_ (info), next_state_ (INIT)
         {
         }

         void run (const iofwdevent::CBException & e);

      protected:
         RPCServer &          server_;
         net::Net::AcceptInfo info_;
         State                next_state_;
         const void *         read_ptr_;
         size_t               read_size_;
         RPCHandler           handler_;
         RPCHeader            header_;
         RPCResponse          response_;
         void *               write_ptr_;
         size_t               write_size_;
   };

   void RPCServer::RPCHelper::run (const iofwdevent::CBException & e)
   {
      e.check ();
      while (true)
      {
         switch (next_state_)
         {
            case INIT:
               next_state_ = READ_READY;
               info_.in->read (&read_ptr_, &read_size_,
                     boost::bind (&RPCServer::RPCHelper::run, this, _1), 0);
               return;
            case READ_READY:
               {
                  RPCDecoder dec (read_ptr_, read_size_);
                  process (dec, header_);
                  next_state_ = REWIND_READY;
                  info_.in->rewindInput (read_size_ - dec.getPos (),
                     boost::bind (&RPCServer::RPCHelper::run, this, _1));
               }
               return;
            case REWIND_READY:
               {
                  try
                  {
                     handler_ = server_.lookupHandler (header_.key);
                     response_.status = RPCResponse::RPC_OK;
                  }
                  catch (UnknownRPCKeyException & e)
                  {
                     ZLOG_INFO (server_.log_, "Unknown RPC request. Ignoring.");
                     response_.status = RPCResponse::RPC_UNKNOWN;
                  }
                  // Send response
                  next_state_ = WRITE_READY;
                  info_.out->write (&write_ptr_, &write_size_,
                     boost::bind (&RPCServer::RPCHelper::run, this, _1), 0);
                  return;
               }
            case WRITE_READY:
               {
                  RPCEncoder enc (write_ptr_, write_size_);
                  process (enc, response_);
                  const size_t used = enc.getPos ();
                  next_state_ = WRITE_REWIND_READY;
                  info_.out->rewindOutput (write_size_ - used,
                     boost::bind (&RPCServer::RPCHelper::run, this, _1));
                  return;
               }
            case WRITE_REWIND_READY:
               {
                  // Execute
                  // make copy of streams so we can delete this
                  iofwdevent::ZeroCopyInputStream * in = info_.in;
                  iofwdevent::ZeroCopyOutputStream * out = info_.out;
                  RPCInfo info;
                  RPCHandler handler;
                  handler.swap (handler_);
                  delete this;

                  handler (in, out, info);
                  return;
               }
            default:
               ALWAYS_ASSERT(false);
         }
      }
   }

   // =======================================================================
   // =======================================================================
   // =======================================================================

   RPCServer::RPCServer (RPCRegistry & r)
      : registry_(r),
        log_ (iofwdutil::IOFWDLog::getSource ("rpcserver"))
   {
   }

   const RPCHandler & RPCServer::lookupHandler (const RPCKey & key) const
   {
      // Registry is thread-safe
      return registry_.lookupFunction (key);
   }

   /**
    * Decode RPCKey and call the correct function
    */
   net::Net::AcceptHandler RPCServer::getHandler ()
   {
      return boost::bind (&RPCServer::rpcHandler, this, _1);
   }

   /**
    * Decode header, make sure we're dealing with an RPC call,
    * lookup key in registry, wrap input/output for transformation if needed
    * and call the correct RPCHandler function.
    */
   void RPCServer::rpcHandler (const net::Net::AcceptInfo & info)
   {
      RPCHelper * h = new RPCHelper (*this, info);
      h->run (iofwdevent::CBException ());
   }

   RPCServer::~RPCServer ()
   {
   }

   //========================================================================
}

