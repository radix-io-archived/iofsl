#ifndef IOFWDUTIL_GENERICTRANSFORM_HH
#define IOFWDUTIL_GENERICTRANSFORM_HH

#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"
#include "iofwdutil/FactoryAutoRegister.hh"
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
      TRANSFORM_DONE,           // returned when flush is set and all data has
                                // been processed (both input and output). 
                                // TRANSFORM_DONE implies CONSUME_OUTBUF

      GENERIC_TRANSFORM_LAST    // This is not a state, it is used to
                                // the first free state code.
    };

    /**
     * Interface for generic transformations. Example, encryption,
     * compression, decompression, ...
     *
     * @TODO Add setHint (std::string) method to allow for transform specific
     * options (such as compression level)
     * @TODO Get rid of getTransformState function
     */
    class GenericTransform
    {
      public:
         // @TODO: add getName () function
         
        FACTORY_CONSTRUCTOR_PARAMS();

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

    // Tags for factory
    struct GTEncode {};
    struct GTDecode {};

    // @TODO: We should hide that this is using a factory underneath.
    //        SHould not throw NoSuchFactoryKey exception or anything like
    //        that.
    typedef iofwdutil::Factory<std::string,GenericTransform,GTEncode>
       GenericTransformEncodeFactory;
    
    typedef iofwdutil::Factory<std::string,GenericTransform,GTDecode>
       GenericTransformDecodeFactory;

     //======================================================================
  }

}

#endif
