
#ifndef IOFWDUTIL_ZOIDFS_LZFCOMPRESS_HH
#define IOFWDUTIL_ZOIDFS_LZFCOMPRESS_HH

#include "GenericTransform.hh"
#include "src/encoder/xdr/XDRWriter.hh"
#include "IOWriteBuffer.hh"
#include "IOReadBuffer.hh"
#include "LZFCommon.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class LZFCompress : public GenericTransform, protected LZFCommon
      {
         public:
            LZFCompress ();

            virtual ~LZFCompress ();


            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:

            encoder::xdr::XDRWriter writer_;

            size_t block_size_;
            bool   block_compressed_;
            state  curstate_;

            const char * compress_input_;
            char * compress_output_;
            size_t compress_outsize_;
            size_t compress_insize_;

            bool update_in_;
            bool update_out_;

            enum { COMPRESSING_GETINPUT = 0,
                   COMPRESSING_GETOUTPUT,
                   COMPRESSING,
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
