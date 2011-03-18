#ifndef SRC_IOFSL_ENCODERSTRING_HH
#define SRC_IOFSL_ENCODERSTRING_HH

#include <stdint.h>
#include "Util.hh"
#include "Size.hh"
#include "EncoderException.hh"
#include "EncoderWrappers.hh"
namespace encoder {
   template <size_t MINSIZE, size_t MAXSIZE>
   struct EncoderString
   {
      std::string value;
   };

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
         typename only_size_processor<ENC>::type * )
   {
      uint32_t len = p.value.size();
      process (e, len);
      process (e, EncOpaque (p.value.c_str(), len,  MAXSIZE));
   }

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                        typename only_decoder_processor<ENC>::type *)
   {
      uint32_t len;
      process (e, &len);      

      if (len < MINSIZE || len > MAXSIZE)
      {
         ZTHROW (BufferException() << iofwdutil::zexception_msg (
          "Invalid length (ether too large or too small for EncoderString)"));
      }

      p.value.resize (len)
      process (e, EncOpaque (p.value.c_str(), len, MAXSIZE));
   }

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                   typename only_encoder_processor<ENC>::type *)
   {
      /* Get the encoder and string length */
      uint32_t stringLen = p.value.size();
   
      if (stringLen < MINSIZE || stringLen > MAXSIZE)
      {
         ZTHROW (BufferException() << iofwdutil::zexception_msg (
     "Invalid hint length (ether too large or too small for EncoderString)"));
      }

      process (e, stringLen);
      process (e, EncOpaque (p.value.c_str(), stringLen, MAXSIZE));
   }
}
#endif
