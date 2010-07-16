#ifndef IOFWDUTIL_GENERICTRANSFORM_HH
#define IOFWDUTIL_GENERICTRANSFORM_HH

#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"
#include "iofwdutil/FactoryAutoRegister.hh"
#include <boost/mpl/list.hpp>
#include <string>


namespace iofwdutil
{

  namespace transform
  {
     //======================================================================

    enum state
    {
      CONSUME_OUTBUF = 100,
      SUPPLY_INBUF,

      GENERIC_TRANSFORM_LAST
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
        virtual state getTransformState () const = 0;


        virtual ~GenericTransform ();
    };

     //======================================================================

    typedef iofwdutil::Factory<std::string,GenericTransform>
       GenericTransformFactory;

     //======================================================================
  }

}

#endif
