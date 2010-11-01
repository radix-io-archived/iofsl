#ifndef IOFWDUTIL_IOFWDZLIB_HH
#define IOFWDUTIL_IOFWDZLIB_HH


extern "C"
{
#include <zlib.h>
}

#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{

  namespace transform
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

        virtual state getTransformState () const;

        virtual ~ZLib();
    };

    //=======================================================================
  }

}

#endif
