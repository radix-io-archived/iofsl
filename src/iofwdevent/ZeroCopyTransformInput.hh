/**
 * @class ZeroCopyTransformInput
 * 
 * @brief Zero copy input stream implementation for creating a stream
 *        from a transformed input (ex: compressed input).
 *
 * This class allows for a ZeroCopyInputStream to be created using a compressed
 * ZeroCopyMemoryInput stream. 
 *
 */
#include "ZeroCopyInputStream.hh"
#include "ZeroCopyMemoryInput.hh"
#include "ZeroCopyMemoryOutput.hh"
#include "iofwdutil/transform/GenericTransform.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include <deque>
#include <boost/smart_ptr.hpp>

using namespace iofwdutil;
using namespace boost;
namespace iofwdevent {
  /**
   * Creates an input zero copy stream from a memory region 
   *
   * @TODO: Need to define semantics: does this class own the transform and
   * the input stream? If so, should be using scoped_ptr to avoid memory
   * leaks.
   *
   * If not: should be documented
   */
  class ZeroCopyTransformInput : public ZeroCopyInputStream {
    typedef iofwdutil::transform::GenericTransform GenericTransform;
    protected:
      ZeroCopyMemoryInput * streamStorage;
      ZeroCopyInputStream * stream; /*< Stores input stream from which to transform */
      GenericTransform * transform; /*< Stores transform information */
      void * inStreamData; /*< Memory pointer to internal stream read (stream) */
      size_t inStreamSize; /*< Input size */
      void * buff;         /*< used by stream storeage as its internal buffer */
      size_t buffSize;     /*< the internal buffer size (maximum) */
      bool flushFlag;
      static const int SUPPLY_INBUF = iofwdutil::transform::SUPPLY_INBUF;
      static const int TRANSFORM_DONE = iofwdutil::transform::TRANSFORM_DONE;
      static const int CONSUME_OUTBUF = iofwdutil::transform::CONSUME_OUTBUF;
    public:
      /**
       * Constructor for ZeroCopyTransformInput.
       * @param[in] in        Transformed input stream which needs to be 
       *                      transformed back into its original stream.
       */
      ZeroCopyTransformInput  (ZeroCopyInputStream * in, GenericTransform * transform,
                               size_t bufSize = 4194304);

      /**
       * Cancel the transformation operation specified by Handle x.
       * @param[in] x         Handle of operation to stop
       */
      void cancel (Handle x) { x = (Handle) 0; }; 

      ~ZeroCopyTransformInput ();
      /**
       * Read from the input stream and preform the transform from 
       * transformed input -> output.
       * @param[out] out       Pointer to location where output is going to be 
       *                       stored.
       * @param[out] len       size of out (actual)
       * @param[in]  cb        Callback to call when operation is complete 
       * @param[in]  suggested Size the client would like to see returned when 
       *                       read operation is completed. This is a best
       *                       effort and the actual size may vary from 
       *                       suggested.
       */
      Handle read (const void **, size_t * , const CBType &, size_t );
  
      /**
       * Return undigested input back to the stream 
       * @param[in] len        Size of data that is being returned to the stream
       * @param[in] cb         Callback when operation is complete.
       */
      Handle rewindInput (size_t , const CBType & );

      /* Read the transform storage stream */
      void readTransformStorage (const void **, size_t *, const CBType &, 
                                 size_t);

      /* Perform the transformation */
      void doTransform (const void **, size_t *, const CBType &, 
                        size_t);

      
      Handle readStream (const void **, size_t *, const CBType &,  size_t);
      void nullCB (CBException e);
      void transformationState (CBException, const void ** , size_t * , 
                                const CBType & , size_t , size_t *, void **);
  };
}


