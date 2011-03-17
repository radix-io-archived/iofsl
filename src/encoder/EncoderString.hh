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
         typename only_size_processor<ENC>::type * );

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                        typename only_decoder_processor<ENC>::type *);

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                   typename only_encoder_processor<ENC>::type *);
}
#endif
