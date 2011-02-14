#ifndef NET_BMI_BMIOUTPUTSTREAM_HH
#define NET_BMI_BMIOUTPUTSTREAM_HH

#include "iofwdevent/ZeroCopyOutputStream.hh"
#include "iofwdevent/Resource-fwd.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

#include "BMIOutput.hh"
#include "BMIStreamHeaders.hh"

namespace net
{
   namespace bmi
   {
      //=====================================================================

      class BMIOutputStream : public iofwdevent::ZeroCopyOutputStream
      {
         public:
            BMIOutputStream (iofwdutil::IOFWDLogSource & log,
                  iofwdevent::BMIResource & bmi,
                  BMI_addr_t addr,
                  bmi_msg_tag_t tag,
                  bool incoming,
                  const BMIStreamHeader & h);

            virtual ~BMIOutputStream ();

         protected:
            void writeComplete (void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e);
      
            void doAlloc (void ** ptr, size_t * size, size_t suggested);

            void flushAndAlloc (void ** ptr, size_t * size, size_t suggested,
                  const iofwdevent::CBType & cb,
                  const iofwdevent::CBException & e);

         public:
            iofwdevent::Handle write (void ** ptr, size_t * size,
                  const iofwdevent::CBType & cb, size_t suggested);

            iofwdevent::Handle rewindOutput (size_t size,
                  const iofwdevent::CBType & cb);

            iofwdevent::Handle flush (const iofwdevent::CBType & cb);

         protected:
            BMIOutput output_;
            void * curbuf_;
            size_t curbuf_size_;
            size_t curbuf_used_;
      };

      //=====================================================================
   }
}

#endif

