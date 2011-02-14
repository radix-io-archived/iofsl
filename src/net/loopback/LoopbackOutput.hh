#ifndef NET_LOOPBACK_LOOPBACKOUTPUT_HH
#define NET_LOOPBACK_LOOPBACKOUTPUT_HH

#include "iofwdevent/ZeroCopyOutputStream.hh"
#include "MessageQueue.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      class LoopbackConnector;

      class LoopbackOutput : public iofwdevent::ZeroCopyOutputStream
      {
         protected:
            friend class LoopbackConnector;

            LoopbackOutput (MessageQueuePtr ptr, size_t def_blocksize,
                  size_t max_blocksize);

         public:

            iofwdevent::Handle write (void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb, size_t suggested);

            iofwdevent::Handle rewindOutput (size_t size,
                  const iofwdevent::CBType & cb);

            iofwdevent::Handle flush (const iofwdevent::CBType & cb);

            virtual ~LoopbackOutput ();

         protected:
            void internal_flush ();

         protected:

            MessageQueuePtr queue_;
            size_t def_blocksize_;
            size_t max_blocksize_;

            void * thisblock_ptr_;
            size_t thisblock_size_;
            size_t thisblock_used_;
      };

      //=====================================================================
   }
}

#endif
