#ifndef NET_BMI_BMISTREAMHEADERS_HH
#define NET_BMI_BMISTREAMHEADERS_HH

#include <stdint.h>
#include <cstring>

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"

namespace net
{
   namespace bmi
   {
      //=====================================================================

      /**
       * BMI Stream header (not per-packet header)
       */
      struct BMIStreamHeader
      {
         typedef encoder::xdr::XDRWriter Encoder;
         typedef encoder::xdr::XDRReader Decoder;
         typedef encoder::xdr::XDRSizeProcessor SizeProcessor;

         enum {
            FLAG_COMPRESSED = 0x01,
            FLAG_CHECKSUM   = 0x02
         };
         char magic[4];
         uint32_t version;
         uint32_t cookie;
         uint32_t packet_size;    // max packet size on this connection
         uint32_t flags;

         size_t decode (const void * buffer, size_t maxsize);
         size_t encode (void * buffer, size_t maxsize);

         static size_t getEncodedSize ();
      };

      struct BMIPacketHeader
      {
         typedef encoder::xdr::XDRWriter Encoder;
         typedef encoder::xdr::XDRReader Decoder;
         typedef encoder::xdr::XDRSizeProcessor SizeProcessor;

         char magic[4];
         uint32_t cookie;
         uint32_t seq;

         size_t decode (const void * buffer, size_t maxsize);
         size_t encode (void * buffer, size_t maxsize);

         static size_t getEncodedSize ();
      };

      //=====================================================================
   }
}

#endif
