#include "BMIStreamHeaders.hh"
#include "src/encoder/EncoderWrappers.hh"

using encoder::EncOpaque;

namespace net
{
   namespace bmi
   {
      //=====================================================================

      static char BMI_STREAM_MAGIC[4] = { 0x66, 0x67, 0x68, 0x69 };

      template <typename T>
      void process (T & enc, BMIStreamHeader & b)
      {
         process (enc, EncOpaque (&b.magic, sizeof(b.magic)));
         process (enc, b.version);
         process (enc, b.cookie);
         process (enc, b.packet_size);
         process (enc, b.flags);
      }

      template <typename T>
      void process (T & enc, BMIPacketHeader & b)
      {
         process (enc, EncOpaque (&b.magic, sizeof(b.magic)));
         process (enc, b.cookie);
         process (enc, b.seq);
      }

      // --------------------------------------------------------------------

      size_t BMIStreamHeader::encode (void * buffer, size_t maxsize)
      {
         ALWAYS_ASSERT(sizeof (magic) == sizeof (BMI_STREAM_MAGIC));
         memcpy (&magic[0], &BMI_STREAM_MAGIC[0], sizeof (magic));

         Encoder enc (buffer, maxsize);
         process (enc, *this);
         return enc.getPos ();
      }


      size_t BMIStreamHeader::decode (const void * buffer, size_t maxsize)
      {
         Decoder dec (buffer, maxsize);
         process (dec, *this);

         // check magic
         ALWAYS_ASSERT(memcmp (&magic[0], &BMI_STREAM_MAGIC[0],
                  sizeof(magic)) == 0);

         return dec.getPos ();
      }

      size_t BMIStreamHeader::getEncodedSize ()
      {
         BMIStreamHeader h;
         SizeProcessor s;
         process (s, h);
         return s.size ().getMaxSize ();
      }

      // --------------------------------------------------------------------

      size_t BMIPacketHeader::decode (const void * buffer, size_t maxsize)
      {
         Decoder dec (buffer, maxsize);
         process (dec, *this);

         // check magic
         ALWAYS_ASSERT(memcmp (&magic[0], &BMI_STREAM_MAGIC[0],
                  sizeof(magic)) == 0);

         return dec.getPos ();
      }
      
      size_t BMIPacketHeader::encode (void * buffer, size_t maxsize)
      {
         ALWAYS_ASSERT(sizeof (magic) == sizeof (BMI_STREAM_MAGIC));
         memcpy (&magic[0], &BMI_STREAM_MAGIC[0], sizeof (magic));

         Encoder enc (buffer, maxsize);
         process (enc, *this);
         return enc.getPos ();
      }

      size_t BMIPacketHeader::getEncodedSize ()
      {
         BMIPacketHeader h;
         SizeProcessor s;
         process (s, h);
         return s.size ().getMaxSize ();
      }

      //=====================================================================
   }
}
