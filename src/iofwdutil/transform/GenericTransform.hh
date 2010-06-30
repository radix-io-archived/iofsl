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
     //======================================================================

    enum state
    {
      CONSUME_OUTBUF = 100,
      SUPPLY_INBUF,
      TRANSFORM_STREAM_ERROR,
      TRANSFORM_STREAM_END
    };

    /**
     * Interface for generic transformations. Example, encryption,
     * compression, decompression, ...
     */
    class GenericTransform
    {
      public:
        typedef boost::mpl::list<> FACTORY_SIGNATURE;

        /**
         * Reset the tranformation engine so that a new stream can be
         * transformed.
         */
        virtual void reset () = 0;

        /**
         * Feed more data into the transformation engine.
         */
        virtual void transform(const void *const inBuf, size_t inSize,
                               void *outBuf, size_t outSize, size_t *outBytes,
                               int *outState, bool flushFlag) = 0;

        /**
         * Return state of the transformation engine
         */
        virtual state getDecompressState() const = 0;


        virtual ~GenericTransform ();
    };

     //======================================================================
  }

}

#endif
