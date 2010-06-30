#ifndef IOFWDUTIL_ZLIB_HH
#define IOFWDUTIL_ZLIB_HH

#include <cstdio>
#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{

  namespace iofwdtransform
  {
    //=======================================================================

    class ZLib : public GenericTransform
    {
      protected:
        state    decompress_state;
        bool     outbuf_partially_filled;
        z_stream stream;

      public:
        ZLib();

        virtual void reset () { ALWAYS_ASSERT(false && "not implemented"); }

        virtual void transform(const void *const inBuf, size_t inSize,
                      void *outBuf, size_t outSize, size_t *outBytes,
                      int *outState, bool flushFlag);

        virtual state getDecompressState() const;

        virtual ~ZLib();
    };

    //=======================================================================
  }

}

#endif
