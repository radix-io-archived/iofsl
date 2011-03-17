#include "EncoderString.hh"

namespace encoder {

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
         typename only_size_processor<ENC>::type * )
   {
      uint32_t len = p.value.size ();
      process (e, len);   // adds size of encoded uint32_t
      process (e, EncOpaque (p.value.c_str(), MAXSIZE));
      
   }

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                        typename only_decoder_processor<ENC>::type *)
   {
      std::string tmp;
      uint32_t len;
      process (e, &len);      
      tmp.resize (len);

      if (len > MINSIZE && len < MAXSIZE)
      {
         process (e, EncString (tmp.c_str(), MAXSIZE));
         p.value.resize(tmp.size());
         tmp.copy(p.value.c_str(), tmp.size(), 0);           
      }
      else
         ZTHROW (BufferException() << iofwdutil::zexception_msg (
     "Invalid hint length (ether too large or too small for EncoderString)"));
      
   }

   template <typename ENC, size_t MINSIZE, size_t MAXSIZE>
   static void process (ENC e, EncoderString<MINSIZE,MAXSIZE> & p,
                   typename only_encoder_processor<ENC>::type *)
   {
      /* Get the encoder and string length */
      uint32_t stringLen = p.value.size();
   
      if (stringLen > MINSIZE && stringLen < MAXSIZE)
      {
         process (e, stringLen);
         process (e, EncString (p.value.c_str(), stringLen));
      }
      else
         ZTHROW (BufferException() << iofwdutil::zexception_msg (
     "Invalid hint length (ether too large or too small for EncoderString)"));
   }
}
