#ifndef IOFWDUTIL_GENERICTRANSFORM_HH
#define IOFWDUTIL_GENERICTRANSFORM_HH

#include <zlib.h>
#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"
#include "iofwdutil/FactoryAutoRegister.hh"
#include <boost/mpl/list.hpp>
#include <string>


namespace iofwdutil
{

  namespace iofwdtransform
  {

    enum
    {
      CONSUME_OUTBUF = 10,
      SUPPLY_INBUF,
      TRANSFORM_STREAM_ERROR,
      TRANSFORM_STREAM_END
    };

    class GenericTransform
    {
      protected:

      public:
	virtual void transform(const void *const inBuf, const size_t inSize,
			       void *outBuf, const size_t outSize, size_t *outBytes,
			       int *outState, const bool flushFlag) = 0;
	typedef boost::mpl::list<> FACTORY_SIGNATURE;
    };

  }

}

#endif
