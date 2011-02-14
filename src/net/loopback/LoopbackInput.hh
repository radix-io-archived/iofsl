#ifndef NET_LOOPBACK_LOOPBACKINPUT_HH
#define NET_LOOPBACK_LOOPBACKINPUT_HH

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "MessageQueue.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      class LoopbackConnector;

      struct LoopbackInput : public iofwdevent::ZeroCopyInputStream
      {
         protected:
            friend class LoopbackConnector;

            LoopbackInput (MessageQueuePtr queue);

         public:

            // Return a pointer to data
            iofwdevent::Handle read (const void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb, size_t suggested);

            // Put back some data into the stream
            iofwdevent::Handle rewindInput (size_t size,
                  const iofwdevent::CBType & cb);

            virtual ~LoopbackInput ();

         protected:
            void getComplete (const iofwdevent::CBException & e);

         protected:
            MessageQueuePtr queue_;

            const void **      get_ptr_;
            size_t *           get_size_;

            const void *       rewindptr_;
            size_t             rewindsize_;
            size_t             rewindused_;

            iofwdevent::CBType cb_;
      };

      //=====================================================================
   }
}

#endif
