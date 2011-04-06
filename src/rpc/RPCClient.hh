#ifndef RPC_RPCCLIENT_HH
#define RPC_RPCCLIENT_HH

#include "iofwdevent/CBType.hh"
#include "net/Net.hh"
#include "RPCKey.hh"

#include "iofwdutil/IntrusiveHelper.hh"
#include "iofwdutil/transform/GenericTransform.hh"

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
               const net::Connection & con, const char * options);

      public:

         /**
          * Make an outgoing RPC call over the specified connection.
          * This might modify the stream objects (i.e. change the
          * net::Connection in/out pointers)
          *  
          * Ownership of Con transfers to RPCClient
          *
          * @TODO: add cancel
          * @TODO: reuse ZoidFS hints for Net framework?
          */
         static RPCClientHandle rpcConnect (const RPCKey & key,
                 const net::Connection & con, const char * options = 0);

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

         /**
          * Return the output stream. Can only be called after waitOutReady
          * completes.
          */
         iofwdevent::ZeroCopyOutputStream * getOut ()
         { return transout_ ? transout_ : con_.out; }

         /**
          * Return the input stream. Can only be called after waitInReady
          * _and_ waitOutReady completes.
          */
         iofwdevent::ZeroCopyInputStream * getIn ()
         { return transin_ ? transin_ : con_.in; }

      protected:
         void startWrite ();

         void writeReady (const iofwdevent::CBException & e);

         void flushHeader (const iofwdevent::CBException & e);

         void rewindReady (const iofwdevent::CBException & e);

         void readReady (const iofwdevent::CBException & e);

         void handleError (uint32_t code);

         void readRewindReady (const iofwdevent::CBException & e);
         void processOption (const std::string & s);

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

         uint32_t flags_out_;
         uint32_t flags_in_;

         iofwdevent::ZeroCopyOutputStream * transout_;
         iofwdevent::ZeroCopyInputStream * transin_;
   };

   INTRUSIVE_PTR_HELPER(RPCClient);


   //========================================================================
}
#endif
