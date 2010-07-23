#ifndef IOFWDUTIL_ZOIDFS_LZF_HH
#define IOFWDUTIL_ZOIDFS_LZF_HH

#include "GenericTransform.hh"
#include "src/encoder/xdr/XDRReader.hh"
#include "IOWriteBuffer.hh"
#include "IOReadBuffer.hh"
#include "LZFCommon.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZF : public GenericTransform, protected LZFCommon
      {
         // @TODO: get this info from common header with C lzf stream code
         // @TODO: if we get another compression library without
         // blocking/stream support, make this class a base and derive
         // implementing just the call to the stateless compress/decompress
         // function. (i.e. sharing the blocking/header scheme)

         public:
            LZF ();

            virtual ~LZF ();


            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:

            encoder::xdr::XDRReader reader_;

            size_t block_size_;
            bool   block_compressed_;
            state  curstate_;
            const void * decompress_input_;
            void * decompress_output_;
            size_t output_size_;

            bool update_in_;
            bool update_out_;

            enum { READING_HEADER = 0,
                   DECOMPRESSING_GETINPUT,
                   DECOMPRESSING_GETOUTPUT,
                   DECOMPRESSING,
                   COPYING,
                   FLUSH_INPUT,
                   FLUSH_OUTPUT,
                   FLUSHED
                 };
            int    internal_state_;


            // ==============================================================
            IOReadBuffer iostate_read_;
            IOWriteBuffer iostate_write_;
      };

      //=====================================================================
   }
}
#endif
