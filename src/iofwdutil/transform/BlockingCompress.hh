#ifndef IOFWDUTIL_ZOIDFS_BLOCKINGCOMPRESS_HH
#define IOFWDUTIL_ZOIDFS_BLOCKINGCOMPRESS_HH

#include "GenericTransform.hh"
#include "src/encoder/xdr/XDRWriter.hh"
#include "IOWriteBuffer.hh"
#include "IOReadBuffer.hh"
#include "BlockingCommon.hh"
#include <cstddef>


namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      /**
       * Base class for compression methods that do not offer their own stream
       * methods.
       *
       * Tries to accumulate a full block of data, compress and stores the
       * compressed form is it is smaller than the uncompressed data.
       */
      class BlockingCompress : public GenericTransform,
                               protected BlockingCommon
      {
         public:
            BlockingCompress ();

            virtual ~BlockingCompress ();

            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:

            /// Compress from in to out, writing up to outsize bytes. Returns
            /// Return numbers of bytes written to out
            /// Should return 0 if the output buffer was too small to hold the
            /// compressed data.
            virtual size_t doCompress (const void * in, size_t insize,
                  void * out, size_t outsize) = 0;

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
