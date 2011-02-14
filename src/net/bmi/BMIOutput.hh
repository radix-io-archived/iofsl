#ifndef NET_BMI_BMIOUTPUT_HH
#define NET_BMI_BMIOUTPUT_HH

#include "iofwdevent/CBType.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdevent/Resource-fwd.hh"

#include "BMIStreamHeaders.hh"


extern "C" {
#include <bmi.h>
}

namespace net
{
   namespace bmi
   {
      //=====================================================================

      /**
       * @TODO:
       *    We are only forced to send on a flush, so in order to improve
       *    performance of the network we could actually aggregate multiple
       *    buffers (allocated through allocWrite) and only send them once we
       *    get up to a decent amount.
       *    That way we don't waist memory by enforcing a buf memory buffer
       *    and we don't limit the send size. Note that since there is no
       *    probe in BMI, we will have to allocate the full max size on the
       *    receiving end.
       */
      class BMIOutput
      {
         public:
            // Constructor for client connections (outgoing)
            BMIOutput (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  BMI_addr_t endpoint,
                  bmi_msg_tag_t tag,
                  bool incoming,
                  const BMIStreamHeader & h);

            ~BMIOutput ();

            // ---------- writing ---------

            /// Allocates write buffer, and returns pointer and size available
            void allocWrite (void ** ptr, size_t * size, size_t suggested);

            /**
             * Write size bytes of the current write buffer. Write buffer is
             * automatically released. As soon as write returns, a new write
             * buffer can be allocated and filled.
             */
            void flush (size_t size, const iofwdevent::CBType & cb);

         protected:
            size_t getMaxSend () const;

            // Called when post_send completes
            void writeDone (void * sendptr, size_t allocsize,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e);

         protected:
            iofwdutil::IOFWDLogSource & log_;
            iofwdevent::BMIResource & bmiresource_;

            BMI_addr_t    endpoint_;
            bmi_msg_tag_t tag_;

            // If we already sent something on this connection
            bool          send_open_;

            // Cookie for this connection
            uint32_t cookie_;

            // Max packet size on this connection
            uint32_t max_packet_;

            // Current send buffer (not user ptr)
            void * cur_send_;
            size_t cur_send_size_;
            size_t cur_send_used_;

            // Max BMI packet sizes
            size_t max_unexpected_packet_;
            size_t max_expected_packet_;

            uint16_t seq_;

            // For empty-packet elimination
            bool reuse_;
      };

      //=====================================================================
   }
}

#endif
