#ifndef IOFWDUTIL_TRANSFORM_COPYTRANSFORM_HH
#define IOFWDUTIL_TRANSFORM_COPYTRANSFORM_HH

#include "GenericTransform.hh"
#include "IOWriteBuffer.hh"
#include "IOReadBuffer.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class CopyTransform : public GenericTransform
      {
         enum { BLOCKSIZE = 64*1024 };

         public:
            CopyTransform ();

            virtual ~CopyTransform ();


            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:

            size_t block_size_;
            state  curstate_;

            const void * copy_input_;
            void * copy_output_;

            bool update_in_;
            bool update_out_;

            enum { GETINPUT,
                   GETOUTPUT,
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
