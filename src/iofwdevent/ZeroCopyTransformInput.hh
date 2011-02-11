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
#include "iofwdutil/transform/GenericTransform.hh"
#include "CBType.hh"
#include "Handle.hh"
#include "CBException.hh"
#include <boost/smart_ptr.hpp>

using namespace iofwdutil;
using namespace boost;
namespace iofwdevent {
  /**
   * Creates an input zero copy stream from a memory region 
   */
  class ZeroCopyTransformInput : public ZeroCopyInputStream {
    typedef iofwdutil::transform::GenericTransform GenericTransform;
    protected:
      ZeroCopyInputStream * mem; /*< Stores input stream from which to transform */
      GenericTransform * transform; /*< Stores transform information */
      size_t memSize;          /*< Stores the size of the memory location */
      size_t pos;              /*< Stores current pointer position inside mem */

    public:
      /**
       * Constructor for ZeroCopyTransformInput.
       * @param[in] in        Transformed input stream which needs to be 
       *                      transformed back into its original stream.
       */
      ZeroCopyTransformInput  (ZeroCopyInputStream *, GenericTransform *);

      /**
       * Cancel the transformation operation specified by Handle x.
       * @param[in] x         Handle of operation to stop
       */
      void cancel (Handle x) {}; 

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
       * Resets the underlying 
      int reset (const void *, size_t);
      /**
       * Return undigested input back to the stream 
       * @param[in] len        Size of data that is being returned to the stream
       * @param[in] cb         Callback when operation is complete.
       */
      Handle rewindInput (size_t , const CBType & );

  };
}


