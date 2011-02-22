#ifndef RPC_RPCCLIENT_HH
#define RPC_RPCCLIENT_HH

#include "iofwdevent/CBType.hh"
#include "net/Net.hh"
#include "RPCKey.hh"

#include "iofwdutil/IntrusiveHelper.hh"

#include <boost/intrusive_ptr.hpp>
#include <boost/thread.hpp>

namespace rpc
{
   //========================================================================

   struct RPCClient;
   typedef boost::intrusive_ptr<RPCClient> RPCClientHandle;

   /**
    * Takes care of adding RPC specific data to input/output stream
    *
    * Maybe (for ease of use) make this refcounted?
    */
   struct RPCClient : public iofwdutil::IntrusiveHelper
   {
      protected:

         RPCClient (const RPCKey & k,
               const net::Connection & con);

      public:

         /**
          * Make an outgoing RPC call over the specified connection.
          * This might modify the stream objects (i.e. change the
          * net::Connection in/out pointers)
          *  
          * Ownership of Con transfers to RPCClient
          *
          * @TODO: add cancel
          */
         static RPCClientHandle rpcConnect (const RPCKey & key,
                 const net::Connection & con);

         /**
          * Calls callback when the output stream becomes available for the
          * user.
          */
         void waitOutReady (const iofwdevent::CBType & cb);

         /**
          * Calls the callback then the input stream becomes available to the
          * user. Note that the output stream might need flushing before the
          * input stream will become ready.
          */
         void waitInReady (const iofwdevent::CBType & cb);

         iofwdevent::ZeroCopyOutputStream * getOut ()
         { return con_.out; }

         iofwdevent::ZeroCopyInputStream * getIn ()
         { return con_.in; }

      protected:
         void startWrite ();

         void writeReady (const iofwdevent::CBException & e);

         void rewindReady (const iofwdevent::CBException & e);

         void readReady (const iofwdevent::CBException & e);

         void handleError (uint32_t code);

         void readRewindReady (const iofwdevent::CBException & e);


      protected:

         boost::mutex lock_;

         RPCKey key_;
         net::Connection con_;

         void * write_ptr_;
         size_t write_size_;
         const void * read_ptr_;
         size_t read_size_;

         bool in_ready_;
         iofwdevent::CBType in_ready_cb_;

         bool out_ready_;
         iofwdevent::CBType out_ready_cb_;
   };

   INTRUSIVE_PTR_HELPER(RPCClient);


   //========================================================================
}
#endif
