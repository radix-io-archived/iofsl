#ifndef IOFWDUTIL_ZOIDFS_BLOCKINGDECOMPRESS_HH
#define IOFWDUTIL_ZOIDFS_BLOCKINGDECOMPRESS_HH

#include "GenericTransform.hh"
#include "src/encoder/xdr/XDRReader.hh"
#include "IOWriteBuffer.hh"
#include "IOReadBuffer.hh"
#include "BlockingCommon.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class BlockingDecompress : public GenericTransform,
                                 protected BlockingCommon
      {

         public:
            BlockingDecompress ();

            virtual ~BlockingDecompress ();


            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:

            /// Returns number of bytes decompressed or 0 if error occured.
            virtual size_t doDecompress (const void * in, size_t insize,
                  void * out, size_t outsize) = 0;

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
