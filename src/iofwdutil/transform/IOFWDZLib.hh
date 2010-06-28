#ifndef IOFWDUTIL_ZLIB_HH
#define IOFWDUTIL_ZLIB_HH

#include "iofwdutil/transform/GenericTransform.hh"
#include <cstdio>

namespace iofwdutil
{

  namespace iofwdtransform
  {

    class ZLib : public GenericTransform
    {
      protected:
	state    decompress_state;
	bool     outbuf_partially_filled;
	z_stream stream;

      public:
	ZLib();
	virtual void transform(const void *const inBuf, const size_t inSize,
		      void *outBuf, const size_t outSize, size_t *outBytes,
		      int *outState, const bool flushFlag);
	virtual state getDecompressState();
        ~ZLib();
    };

  }

}

#endif
