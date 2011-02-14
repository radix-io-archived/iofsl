#ifndef NET_BMI_BMIINPUTSTREAM_HH
#define NET_BMI_BMIINPUTSTREAM_HH

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdevent/Resource-fwd.hh"

#include "BMIInput.hh"
#include "BMIStreamHeaders.hh"

namespace net
{
   namespace bmi
   {
      //=====================================================================

      /**
       * Wraps  BMIInput to provide ZeroCopyInputStream interface.
       * Mainly adds rewind functionality.
       */
      class BMIInputStream : public iofwdevent::ZeroCopyInputStream
      {
         public:

            // Constructor for outgoing connections
            BMIInputStream (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  BMI_addr_t addr,
                  bmi_msg_tag_t tag,
                  const BMIStreamHeader & h);

            // Constructor for incoming connections
            // We get the header from the input stream
            BMIInputStream (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  const BMI_unexpected_info & info,
                  bmi_msg_tag_t tag);


            virtual ~BMIInputStream ();

         public:
            // Return a pointer to data
            iofwdevent::Handle read (const void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb, size_t suggested);

            // Put back some data into the stream
            iofwdevent::Handle rewindInput (size_t size,
                  const iofwdevent::CBType & cb);


         protected:
            void readCompleted (const void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e);

         protected:
            BMIInput     input_;
            const void * curbuf_;
            size_t       curbuf_size_;
            size_t       curbuf_used_;
      };

      //=====================================================================
   }
}

#endif
