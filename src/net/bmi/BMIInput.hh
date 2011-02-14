#ifndef NET_BMI_BMIINPUT_HH
#define NET_BMI_BMIINPUT_HH

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

      class BMIInput
      {
         public:
            // Constructor for client connections (outgoing)
            BMIInput (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  BMI_addr_t endpoint, bmi_msg_tag_t tag,
                  const BMIStreamHeader & h);

            // Constructor for incoming connections
            BMIInput (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  const BMI_unexpected_info & info,
                  bmi_msg_tag_t tag);

            ~BMIInput ();

            /**
             * Return a buffer to read from.
             * Calling this function again will free the previous buffer.
             * Buffer ownership stays with the BMIInput object.
             *
             * If we want more overlap (i.e. preposting reads, ...)
             * we could modify read so that it returns a handle or something
             * which can be passed to a release function.
             */
            void read (const void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb);

         protected:
            void readComplete (const iofwdevent::CBException  & e);

            void postReceive ();

         protected:
            iofwdutil::IOFWDLogSource & log_;
            iofwdevent::BMIResource & bmiresource_;

            BMI_addr_t    endpoint_;
            bmi_msg_tag_t tag_;

            // If we already sent something on this connection
            bool          send_open_;

            // If this was an incoming connection
            BMI_unexpected_info info_;

            // Cookie for this connection
            uint32_t cookie_;

            // Max packet size on this connection
            uint32_t max_packet_;

            // Current read buffer
            void * cur_read_raw_;
            size_t cur_read_raw_size_;   // bytes available in buffer
            size_t cur_read_raw_used_;   // bytes used by header
            size_t cur_read_raw_buffer_size_;
            bmi_size_t cur_read_raw_actual_;
            iofwdevent::CBType cur_read_cb_;
            const void ** user_read_ptr_;
            size_t * user_read_size_;

            uint16_t seq_;
      };

      //=====================================================================
   }
}

#endif
