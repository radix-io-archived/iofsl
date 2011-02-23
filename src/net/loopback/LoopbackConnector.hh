#ifndef NET_LOOPBACKUP_LOOPBACKCONNECTOR_HH
#define NET_LOOPBACKUP_LOOPBACKCONNECTOR_HH

#include "net/Net.hh"

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "iofwdevent/CBType.hh"

namespace net
{
   //========================================================================

   namespace loopback
   {
      //=====================================================================

      class MessageQueue;

      /**
       * Loopback RPC communication connector.
       */
      class LoopbackConnector : private boost::noncopyable,
                                public Net
      {
         protected:

            // Maximum blocksize used to transfer a message
            enum {
               DEFAULT_BLOCKSIZE = 4*1024,
               MAX_BLOCKSIZE = 64*1024*1024
            };

         public:

            LoopbackConnector ();

            ~LoopbackConnector ();

            void lookup (const char * addr, AddressPtr * ptr,
                  const iofwdevent::CBType & cb);

            void setAcceptHandler (const AcceptHandler & h);

            Connection connect (const ConstAddressPtr & addr);
         
            void createGroup (GroupHandle * group,
               const std::vector<std::string> & members,
               const iofwdevent::CBType & cb);

         protected:

            void newQuery (ZeroCopyInputStream * in,
                  ZeroCopyOutputStream * out);

         protected:
            boost::mutex lock_;

            AcceptHandler accept_;
            AddressPtr localhost_;
      };

      //=====================================================================
   }

   //========================================================================
}

#endif
